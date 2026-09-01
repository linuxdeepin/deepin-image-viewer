// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | GlobalStatus | low | - | 1 | 1 |
// | ~GlobalStatus | low | destructor | 1 | 1 |
// | actionMargin | low | - | 1 | 1 |
// | animationBlock | low | - | 1 | 1 |
// | animationDefaultDuration | low | - | 1 | 1 |
// | delayInit | low | - | 1 | 1 |
// | editMode | low | - | 1 | 1 |
// | editModified | low | - | 1 | 1 |
// | enableNavigation | mid | in_degree:3 | 2 | 2 |
// | floatMargin | low | - | 1 | 1 |
// | fullScreenAnimating | low | - | 1 | 1 |
// | minHeight | low | - | 1 | 1 |
// | minHideHeight | low | - | 1 | 1 |
// | minWidth | low | - | 1 | 1 |
// | pathViewItemCount | low | - | 1 | 1 |
// | rightMenuItemHeight | low | - | 1 | 1 |
// | setAnimationBlock | low | - | 1 | 1 |
// | setDelayInit | low | - | 1 | 1 |
// | setEditMode | low | - | 1 | 1 |
// | setEditModified | low | - | 1 | 1 |
// | setEnableNavigation | low | - | 1 | 1 |
// | setFullScreenAnimating | low | - | 1 | 1 |
// | setShowFullScreen | low | - | 1 | 1 |
// | setShowImageInfo | low | - | 1 | 1 |
// | setShowRightMenu | low | - | 1 | 1 |
// | setStackPage | low | - | 1 | 3 |
// | setThumbnailVaildWidth | low | - | 1 | 4 |
// | setViewFlicking | low | - | 1 | 1 |
// | setViewInteractive | low | - | 1 | 1 |
// | showBottomY | low | - | 1 | 1 |
// | showFullScreen | mid | in_degree:3 | 2 | 2 |
// | showImageInfo | low | - | 1 | 1 |
// | showRightMenu | low | - | 1 | 1 |
// | stackPage | low | - | 1 | 1 |
// | switchImageHotspotWidth | low | - | 1 | 1 |
// | thumbnailVaildWidth | low | - | 1 | 1 |
// | thumbnailViewHeight | low | - | 1 | 1 |
// | titleHeight | low | - | 1 | 1 |
// | viewFlicking | low | - | 1 | 1 |
// | viewInteractive | low | - | 1 | 1 |
// ─── actual 均不低于 min；setStackPage/setThumbnailVaildWidth 的 actual 计 TEST_P 实例化数 ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（40/40 方法：构造/析构/12 常量 getter/13 状态 getter/13 状态 setter）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（bool 两值全测；int 宽度 0/360/-1/INT_MAX；
//    StackPage 三枚举值全测 + 默认值）
// 3. 每个等价类的边界值显式覆盖: [x]（thumbnailVaildWidth: 0=默认值、-1 负值、INT_MAX 极值；
//    stackPage: OpenImagePage=默认 0 号页边界；bool: 默认真/默认假两组初值都覆盖）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（11 个 bool 状态属性同构 setter → SetBoolStateProperty ×11；
//    宽度 4 组边界值 → SetThumbnailVaildWidth ×4；3 个页面 → SetStackPage ×3）
// 5. 分支清单 → 用例映射已列出: [x]（见下方 setter 同构分支清单段落；getter/常量/构造/析构均为
//    straight-line 无分支方法，真实分支数 0，§4.1 允许省略）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（13 个 setter 的唯一 if 的真假两侧：
//    变更→emit / 同值→不 emit，均有专门断言）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw，方法体均为状态存取，无异常路径）
// 8. 负面场景有专门用例: [x]（同值 set 不发信号不更新；宽度 0/负值；对默认页 set 同值）
// 9. 负面用例验证强异常安全: [x]（同值分支断言 getter 保持原值且 spy 计数不增长）
// 10. stub_ext vs gMock 选择正确: [x]（本类仅依赖 QObject 信号槽与 qCDebug 日志，无外部
//     IO/DBus/路径/环境变量依赖，无需 stub；Fixture 仍按模板持有 StubExt 并在 TearDown clear）
//
// 状态污染说明：GlobalStatus 全部状态为实例成员（非静态/单例），每用例 SetUp 新建实例、
// TearDown 析构，天然无跨用例/跨文件污染；未使用 qputenv/单例/临时文件，无需额外恢复。

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QVariant>

#include <climits>
#include <memory>
#include <vector>

#include "stub_ext/stubext.h"

#include "globalstatus.h"

namespace {

// 11 个 bool 状态属性的同构描述（来源：globalstatus.h 成员默认值）
// setter 逻辑完全同构：if (value != store*) { store* = value; emit *Changed(); }
//
// ⚠️ 参数结构只携带枚举 id + 默认值，不携带成员指针：曾将 PMF 存入 gtest 堆上
// 参数向量并在用例内 (obj->*c.setter)(...) 运行期间接调用，在 --coverage+ASAN+UBSan
// 插桩构建下代码生成错误（对象基址寄存器经 UBSan 错误路径恢复后被参数派生地址
// 污染，obj 实际从 param+0x90 的相邻堆垃圾装载）→ 全量运行确定性 SEGV。
// 改为 switch 分发到编译期直连调用（与全绿的 TEST_F 同形态）后免疫。
enum class BoolPropId {
    ShowFullScreen,
    EnableNavigation,
    ShowRightMenu,
    ShowImageInfo,
    ViewInteractive,
    ViewFlicking,
    EditMode,
    EditModified,
    AnimationBlock,
    FullScreenAnimating,
    DelayInit,
};

struct BoolPropertyCase {
    const char *name;
    BoolPropId id;
    bool defaultValue;
};

std::vector<BoolPropertyCase> boolPropertyCases()
{
    return {
        { "showFullScreen",      BoolPropId::ShowFullScreen,      false },
        { "enableNavigation",    BoolPropId::EnableNavigation,    true },
        { "showRightMenu",       BoolPropId::ShowRightMenu,       false },
        { "showImageInfo",       BoolPropId::ShowImageInfo,       false },
        { "viewInteractive",     BoolPropId::ViewInteractive,     true },
        { "viewFlicking",        BoolPropId::ViewFlicking,        false },
        { "editMode",            BoolPropId::EditMode,            false },
        { "editModified",        BoolPropId::EditModified,        false },
        { "animationBlock",      BoolPropId::AnimationBlock,      false },
        { "fullScreenAnimating", BoolPropId::FullScreenAnimating, false },
        { "delayInit",           BoolPropId::DelayInit,           true },
    };
}

// switch 分发到编译期直连调用，避免任何运行期成员指针间接寻址
void setBoolState(GlobalStatus *o, BoolPropId id, bool value)
{
    switch (id) {
    case BoolPropId::ShowFullScreen:      o->setShowFullScreen(value);      break;
    case BoolPropId::EnableNavigation:    o->setEnableNavigation(value);    break;
    case BoolPropId::ShowRightMenu:       o->setShowRightMenu(value);       break;
    case BoolPropId::ShowImageInfo:       o->setShowImageInfo(value);       break;
    case BoolPropId::ViewInteractive:     o->setViewInteractive(value);     break;
    case BoolPropId::ViewFlicking:        o->setViewFlicking(value);        break;
    case BoolPropId::EditMode:            o->setEditMode(value);            break;
    case BoolPropId::EditModified:        o->setEditModified(value);        break;
    case BoolPropId::AnimationBlock:      o->setAnimationBlock(value);      break;
    case BoolPropId::FullScreenAnimating: o->setFullScreenAnimating(value); break;
    case BoolPropId::DelayInit:           o->setDelayInit(value);           break;
    }
}

bool boolState(const GlobalStatus *o, BoolPropId id)
{
    switch (id) {
    case BoolPropId::ShowFullScreen:      return o->showFullScreen();
    case BoolPropId::EnableNavigation:    return o->enableNavigation();
    case BoolPropId::ShowRightMenu:       return o->showRightMenu();
    case BoolPropId::ShowImageInfo:       return o->showImageInfo();
    case BoolPropId::ViewInteractive:     return o->viewInteractive();
    case BoolPropId::ViewFlicking:        return o->viewFlicking();
    case BoolPropId::EditMode:            return o->editMode();
    case BoolPropId::EditModified:        return o->editModified();
    case BoolPropId::AnimationBlock:      return o->animationBlock();
    case BoolPropId::FullScreenAnimating: return o->fullScreenAnimating();
    case BoolPropId::DelayInit:           return o->delayInit();
    }
    ADD_FAILURE() << "unreachable bool property id";
    return false;
}

// 每分支用编译期信号 PMF 构造 QSignalSpy（与 TEST_F 相同形态）；unique_ptr 保持生存期
std::unique_ptr<QSignalSpy> makeBoolSpy(GlobalStatus *o, BoolPropId id)
{
    switch (id) {
    case BoolPropId::ShowFullScreen:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::showFullScreenChanged);
    case BoolPropId::EnableNavigation:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::enableNavigationChanged);
    case BoolPropId::ShowRightMenu:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::showRightMenuChanged);
    case BoolPropId::ShowImageInfo:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::showImageInfoChanged);
    case BoolPropId::ViewInteractive:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::viewInteractiveChanged);
    case BoolPropId::ViewFlicking:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::viewFlickingChanged);
    case BoolPropId::EditMode:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::editModeChanged);
    case BoolPropId::EditModified:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::editModifiedChanged);
    case BoolPropId::AnimationBlock:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::animationBlockChanged);
    case BoolPropId::FullScreenAnimating:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::fullScreenAnimatingChanged);
    case BoolPropId::DelayInit:
        return std::make_unique<QSignalSpy>(o, &GlobalStatus::delayInitChanged);
    }
    ADD_FAILURE() << "unreachable bool property id";
    return nullptr;
}

} // namespace

// ═══════════════ 分支清单（来源：get_code_snippet 真实源码）═══════════════
//
// 分支清单（来源：GlobalStatus::setShowFullScreen / setEnableNavigation /
//           setShowRightMenu / setShowImageInfo / setViewInteractive /
//           setViewFlicking / setEditMode / setEditModified /
//           setAnimationBlock / setFullScreenAnimating / setDelayInit /
//           setThumbnailVaildWidth / setStackPage，13 个状态 setter 同构，各 1 个 if）
// B1: value != store*（值变化）→ 更新成员并 Q_EMIT *Changed()
// B2: value == store*（同值）→ 不更新成员、不发信号
// 用例映射：
// - SetShowFullScreen_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetEnableNavigation_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetShowRightMenu_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetShowImageInfo_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetViewInteractive_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetViewFlicking_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetEditMode_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetEditModified_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetAnimationBlock_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetFullScreenAnimating_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetDelayInit_ChangeSameAndRevert_EmitsSignalOnlyOnChange → B1+B2
// - SetThumbnailVaildWidth_BoundaryValues_EmitsOnlyOnChange（TEST_P ×4）→ B1+B2
// - SetStackPage_PageTransitions_EmitsOnlyOnChange（TEST_P ×3）→ B1+B2
// - SetBoolStateProperty_ToggleCycle_EmitsSignalOnlyOnChange（TEST_P ×11，上述
//   11 个 bool setter 的同构回归）→ B1+B2
//
// 其余方法（构造/析构/12 常量 getter/13 状态 getter）方法体无 if/switch/loop/throw/
// 早退 return（get_code_snippet 确认真实分支数 0），§4.1 允许省略分支清单。
// 常量 getter 契约（来源 globalstatus.cpp 文件头 static const）：
//   minHeight=300 minWidth=628 minHideHeight=425 floatMargin=65 titleHeight=50
//   thumbnailViewHeight=70 showBottomY=80 switchImageHotspotWidth=100
//   actionMargin=9 rightMenuItemHeight=32 animationDefaultDuration=366.0
//   pathViewItemCount=3
// 成员默认值（来源 globalstatus.h 私有成员初始化）：
//   showFullScreen=false enableNavigation=true showRightMenu=false
//   showImageInfo=false viewInteractive=true viewFlicking=false editMode=false
//   editModified=false animationBlock=false fullScreenAnimating=false
//   thumbnailVaildWidth=0 stackPage=OpenImagePage delayInit=true
// ═══════════════════════════════════════════════════════════════════════

class GlobalStatusTest : public ::testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        obj = new GlobalStatus();
    }

    void TearDown() override {
        delete obj;   // 状态均为实例成员，析构即恢复，无全局残留
        obj = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    GlobalStatus *obj = nullptr;
};

// TEST_P 参数化子 Fixture（继承主 Fixture，补充 WithParamInterface）
struct GlobalStatusBoolPropTest : public GlobalStatusTest,
                                  public ::testing::WithParamInterface<BoolPropertyCase> {
};
struct GlobalStatusThumbWidthTest : public GlobalStatusTest,
                                    public ::testing::WithParamInterface<int> {
};
struct GlobalStatusStackPageTest : public GlobalStatusTest,
                                   public ::testing::WithParamInterface<Types::StackPage> {
};

// ═══════════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F/TEST_P 均包含 // Arrange / // Act / // Assert 三段注释

// ── 构造 / 析构 ──

TEST_F(GlobalStatusTest, GlobalStatus_Construction_InitializesAllStateDefaultsFromHeader)
{
    // Arrange（SetUp 已 new GlobalStatus() 无父对象；再建一个带父对象的实例验证 parent 传递）
    QObject parent;
    GlobalStatus owned(&parent);

    // Act：读取全部可变状态默认值（构造函数无其它可观察副作用）
    const bool showFullScreen = obj->showFullScreen();
    const bool enableNavigation = obj->enableNavigation();
    const bool showRightMenu = obj->showRightMenu();
    const bool showImageInfo = obj->showImageInfo();
    const bool viewInteractive = obj->viewInteractive();
    const bool viewFlicking = obj->viewFlicking();
    const bool editMode = obj->editMode();
    const bool editModified = obj->editModified();
    const bool animationBlock = obj->animationBlock();
    const bool fullScreenAnimating = obj->fullScreenAnimating();
    const int thumbnailVaildWidth = obj->thumbnailVaildWidth();
    const Types::StackPage stackPage = obj->stackPage();
    const bool delayInit = obj->delayInit();

    // Assert：与 globalstatus.h 私有成员初始化逐一对应
    EXPECT_EQ(obj->parent(), nullptr);
    EXPECT_EQ(owned.parent(), &parent);
    EXPECT_EQ(showFullScreen, false);
    EXPECT_EQ(enableNavigation, true);
    EXPECT_EQ(showRightMenu, false);
    EXPECT_EQ(showImageInfo, false);
    EXPECT_EQ(viewInteractive, true);
    EXPECT_EQ(viewFlicking, false);
    EXPECT_EQ(editMode, false);
    EXPECT_EQ(editModified, false);
    EXPECT_EQ(animationBlock, false);
    EXPECT_EQ(fullScreenAnimating, false);
    EXPECT_EQ(thumbnailVaildWidth, 0);
    EXPECT_EQ(stackPage, Types::OpenImagePage);
    EXPECT_EQ(delayInit, true);
}

TEST_F(GlobalStatusTest, GlobalStatus_Destructor_ParentOwnedInstance_DestroyedWithParent)
{
    // Arrange：堆上父对象 + 父属 GlobalStatus 子对象，双 destroyed 侦听
    QObject *owner = new QObject;
    GlobalStatus *child = new GlobalStatus(owner);
    QSignalSpy spyChild(child, &QObject::destroyed);
    QSignalSpy spyOwner(owner, &QObject::destroyed);

    // Act：删除父对象，~GlobalStatus 随 ~QObject 父子级联被调
    delete owner;

    // Assert：父与子各恰好析构一次（析构本身仅 qCDebug，无其它可观察副作用）
    EXPECT_EQ(spyOwner.count(), 1);
    EXPECT_EQ(spyChild.count(), 1);
}

// ── 常量 getter（Q_PROPERTY ... CONSTANT，值来自 globalstatus.cpp 文件级 sc_* 常量）──

TEST_F(GlobalStatusTest, MinHeight_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_MinHeight = 300， SetUp 已构造 obj）
    const int expected = 300;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->minHeight();
    obj->setShowFullScreen(true);
    const int valueAfterMutation = obj->minHeight();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, MinWidth_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_MinWidth = 628， SetUp 已构造 obj）
    const int expected = 628;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->minWidth();
    obj->setEditMode(true);
    const int valueAfterMutation = obj->minWidth();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, MinHideHeight_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_MinHideHeight = 425， SetUp 已构造 obj）
    const int expected = 425;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->minHideHeight();
    obj->setViewFlicking(true);
    const int valueAfterMutation = obj->minHideHeight();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, FloatMargin_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_FloatMargin = 65， SetUp 已构造 obj）
    const int expected = 65;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->floatMargin();
    obj->setEditModified(true);
    const int valueAfterMutation = obj->floatMargin();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, TitleHeight_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_TitleHeight = 50， SetUp 已构造 obj）
    const int expected = 50;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->titleHeight();
    obj->setShowImageInfo(true);
    const int valueAfterMutation = obj->titleHeight();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, ThumbnailViewHeight_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_ThumbnailViewHeight = 70， SetUp 已构造 obj）
    const int expected = 70;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->thumbnailViewHeight();
    obj->setAnimationBlock(true);
    const int valueAfterMutation = obj->thumbnailViewHeight();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, ShowBottomY_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_ShowBottomY = 80， SetUp 已构造 obj）
    const int expected = 80;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->showBottomY();
    obj->setFullScreenAnimating(true);
    const int valueAfterMutation = obj->showBottomY();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, SwitchImageHotspotWidth_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_SwitchImageHotspotWidth = 100， SetUp 已构造 obj）
    const int expected = 100;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->switchImageHotspotWidth();
    obj->setShowRightMenu(true);
    const int valueAfterMutation = obj->switchImageHotspotWidth();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, ActionMargin_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_ActionMargin = 9， SetUp 已构造 obj）
    const int expected = 9;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->actionMargin();
    obj->setViewInteractive(false);
    const int valueAfterMutation = obj->actionMargin();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, RightMenuItemHeight_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_RightMenuItemHeight = 32， SetUp 已构造 obj）
    const int expected = 32;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->rightMenuItemHeight();
    obj->setDelayInit(false);
    const int valueAfterMutation = obj->rightMenuItemHeight();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, AnimationDefaultDuration_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_AnimationDefaultDuration = 366.0， SetUp 已构造 obj）
    const double expected = 366.0;

    // Act：读常量，翻转无关状态后再读
    const double value = obj->animationDefaultDuration();
    obj->setShowFullScreen(true);
    const double valueAfterMutation = obj->animationDefaultDuration();

    // Assert：CONSTANT double 属性与实例状态无关，两次读值一致
    EXPECT_DOUBLE_EQ(value, expected);
    EXPECT_DOUBLE_EQ(valueAfterMutation, expected);
}

TEST_F(GlobalStatusTest, PathViewItemCount_LayoutConstant_ReturnsScValueStateIndependently)
{
    // Arrange（sc_PathViewItemCount = 3， SetUp 已构造 obj）
    const int expected = 3;

    // Act：读常量，翻转无关状态后再读
    const int value = obj->pathViewItemCount();
    obj->setStackPage(Types::SliderShowPage);
    const int valueAfterMutation = obj->pathViewItemCount();

    // Assert：CONSTANT 属性与实例状态无关，两次读值一致
    EXPECT_EQ(value, expected);
    EXPECT_EQ(valueAfterMutation, expected);
}

// ── 状态 getter（默认值 + setter 写入后读回）──

TEST_F(GlobalStatusTest, ShowFullScreen_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeshowFullScreen = false）
    const bool defaultValue = obj->showFullScreen();

    // Act
    obj->setShowFullScreen(true);
    const bool afterSet = obj->showFullScreen();

    // Assert
    EXPECT_EQ(defaultValue, false);
    EXPECT_EQ(afterSet, true);
}

TEST_F(GlobalStatusTest, ShowFullScreen_QPropertyBridge_ReflectsStateAndDefault)
{
    // Arrange（Q_PROPERTY showFullScreen 供 QML 单例消费）
    QSignalSpy spy(obj, &GlobalStatus::showFullScreenChanged);

    // Act
    const QVariant defaultProperty = obj->property("showFullScreen");
    obj->setShowFullScreen(true);
    const QVariant afterProperty = obj->property("showFullScreen");

    // Assert：属性系统读值与 C++ getter 同步，NOTIFY 只在变更时发射
    EXPECT_EQ(defaultProperty.toBool(), false);
    EXPECT_EQ(afterProperty.toBool(), true);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(GlobalStatusTest, EnableNavigation_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeenableNavigation = true）
    const bool defaultValue = obj->enableNavigation();

    // Act
    obj->setEnableNavigation(false);
    const bool afterSet = obj->enableNavigation();

    // Assert
    EXPECT_EQ(defaultValue, true);
    EXPECT_EQ(afterSet, false);
}

TEST_F(GlobalStatusTest, EnableNavigation_QPropertyBridge_ReflectsStateAndDefault)
{
    // Arrange（Q_PROPERTY enableNavigation 供 QML 单例消费）
    QSignalSpy spy(obj, &GlobalStatus::enableNavigationChanged);

    // Act
    const QVariant defaultProperty = obj->property("enableNavigation");
    obj->setEnableNavigation(false);
    const QVariant afterProperty = obj->property("enableNavigation");

    // Assert：属性系统读值与 C++ getter 同步，NOTIFY 只在变更时发射
    EXPECT_EQ(defaultProperty.toBool(), true);
    EXPECT_EQ(afterProperty.toBool(), false);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(GlobalStatusTest, ShowRightMenu_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeshowRightMenu = false）
    const bool defaultValue = obj->showRightMenu();

    // Act
    obj->setShowRightMenu(true);
    const bool afterSet = obj->showRightMenu();

    // Assert
    EXPECT_EQ(defaultValue, false);
    EXPECT_EQ(afterSet, true);
}

TEST_F(GlobalStatusTest, ShowImageInfo_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeshowImageInfo = false）
    const bool defaultValue = obj->showImageInfo();

    // Act
    obj->setShowImageInfo(true);
    const bool afterSet = obj->showImageInfo();

    // Assert
    EXPECT_EQ(defaultValue, false);
    EXPECT_EQ(afterSet, true);
}

TEST_F(GlobalStatusTest, ViewInteractive_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeviewInteractive = true）
    const bool defaultValue = obj->viewInteractive();

    // Act
    obj->setViewInteractive(false);
    const bool afterSet = obj->viewInteractive();

    // Assert
    EXPECT_EQ(defaultValue, true);
    EXPECT_EQ(afterSet, false);
}

TEST_F(GlobalStatusTest, ViewFlicking_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeviewFlicking = false）
    const bool defaultValue = obj->viewFlicking();

    // Act
    obj->setViewFlicking(true);
    const bool afterSet = obj->viewFlicking();

    // Assert
    EXPECT_EQ(defaultValue, false);
    EXPECT_EQ(afterSet, true);
}

TEST_F(GlobalStatusTest, EditMode_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeEditMode = false）
    const bool defaultValue = obj->editMode();

    // Act
    obj->setEditMode(true);
    const bool afterSet = obj->editMode();

    // Assert
    EXPECT_EQ(defaultValue, false);
    EXPECT_EQ(afterSet, true);
}

TEST_F(GlobalStatusTest, EditModified_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeEditModified = false）
    const bool defaultValue = obj->editModified();

    // Act
    obj->setEditModified(true);
    const bool afterSet = obj->editModified();

    // Assert
    EXPECT_EQ(defaultValue, false);
    EXPECT_EQ(afterSet, true);
}

TEST_F(GlobalStatusTest, AnimationBlock_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeanimationBlock = false）
    const bool defaultValue = obj->animationBlock();

    // Act
    obj->setAnimationBlock(true);
    const bool afterSet = obj->animationBlock();

    // Assert
    EXPECT_EQ(defaultValue, false);
    EXPECT_EQ(afterSet, true);
}

TEST_F(GlobalStatusTest, FullScreenAnimating_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storefullScreenAnimating = false）
    const bool defaultValue = obj->fullScreenAnimating();

    // Act
    obj->setFullScreenAnimating(true);
    const bool afterSet = obj->fullScreenAnimating();

    // Assert
    EXPECT_EQ(defaultValue, false);
    EXPECT_EQ(afterSet, true);
}

TEST_F(GlobalStatusTest, DelayInit_DefaultThenAfterToggle_ReturnsExpectedValues)
{
    // Arrange（默认 storeDelayInit = true，注释要求初始化完成后 MUST 置 false）
    const bool defaultValue = obj->delayInit();

    // Act
    obj->setDelayInit(false);
    const bool afterSet = obj->delayInit();

    // Assert
    EXPECT_EQ(defaultValue, true);
    EXPECT_EQ(afterSet, false);
}

TEST_F(GlobalStatusTest, ThumbnailVaildWidth_DefaultZeroThenAfterSet_ReturnsExpectedValues)
{
    // Arrange（默认 storethumbnailVaildWidth = 0）
    const int defaultValue = obj->thumbnailVaildWidth();

    // Act
    obj->setThumbnailVaildWidth(360);
    const int afterSet = obj->thumbnailVaildWidth();

    // Assert
    EXPECT_EQ(defaultValue, 0);
    EXPECT_EQ(afterSet, 360);
}

TEST_F(GlobalStatusTest, StackPage_DefaultThenPageTransition_ReturnsExpectedPages)
{
    // Arrange（默认 storestackPage = Types::OpenImagePage）
    const Types::StackPage defaultValue = obj->stackPage();

    // Act
    obj->setStackPage(Types::ImageViewPage);
    const Types::StackPage afterSet = obj->stackPage();

    // Assert
    EXPECT_EQ(defaultValue, Types::OpenImagePage);
    EXPECT_EQ(afterSet, Types::ImageViewPage);
}

// ── 状态 setter（B1 变更→emit / B2 同值→不 emit）──

TEST_F(GlobalStatusTest, SetShowFullScreen_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 false）
    QSignalSpy spy(obj, &GlobalStatus::showFullScreenChanged);

    // Act：true（B1 变更）→ true（B2 同值）→ false（B1 复位）
    obj->setShowFullScreen(true);
    const bool afterChange = obj->showFullScreen();
    obj->setShowFullScreen(true);
    const int signalsAfterSameValue = spy.count();
    obj->setShowFullScreen(false);
    const bool afterRevert = obj->showFullScreen();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, true);
    EXPECT_EQ(afterRevert, false);
}

TEST_F(GlobalStatusTest, SetEnableNavigation_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 true）
    QSignalSpy spy(obj, &GlobalStatus::enableNavigationChanged);

    // Act：false（B1 变更）→ false（B2 同值）→ true（B1 复位）
    obj->setEnableNavigation(false);
    const bool afterChange = obj->enableNavigation();
    obj->setEnableNavigation(false);
    const int signalsAfterSameValue = spy.count();
    obj->setEnableNavigation(true);
    const bool afterRevert = obj->enableNavigation();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, false);
    EXPECT_EQ(afterRevert, true);
}

TEST_F(GlobalStatusTest, SetShowRightMenu_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 false）
    QSignalSpy spy(obj, &GlobalStatus::showRightMenuChanged);

    // Act：true（B1 变更）→ true（B2 同值）→ false（B1 复位）
    obj->setShowRightMenu(true);
    const bool afterChange = obj->showRightMenu();
    obj->setShowRightMenu(true);
    const int signalsAfterSameValue = spy.count();
    obj->setShowRightMenu(false);
    const bool afterRevert = obj->showRightMenu();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, true);
    EXPECT_EQ(afterRevert, false);
}

TEST_F(GlobalStatusTest, SetShowImageInfo_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 false）
    QSignalSpy spy(obj, &GlobalStatus::showImageInfoChanged);

    // Act：true（B1 变更）→ true（B2 同值）→ false（B1 复位）
    obj->setShowImageInfo(true);
    const bool afterChange = obj->showImageInfo();
    obj->setShowImageInfo(true);
    const int signalsAfterSameValue = spy.count();
    obj->setShowImageInfo(false);
    const bool afterRevert = obj->showImageInfo();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, true);
    EXPECT_EQ(afterRevert, false);
}

TEST_F(GlobalStatusTest, SetViewInteractive_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 true）
    QSignalSpy spy(obj, &GlobalStatus::viewInteractiveChanged);

    // Act：false（B1 变更）→ false（B2 同值）→ true（B1 复位）
    obj->setViewInteractive(false);
    const bool afterChange = obj->viewInteractive();
    obj->setViewInteractive(false);
    const int signalsAfterSameValue = spy.count();
    obj->setViewInteractive(true);
    const bool afterRevert = obj->viewInteractive();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, false);
    EXPECT_EQ(afterRevert, true);
}

TEST_F(GlobalStatusTest, SetViewFlicking_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 false）
    QSignalSpy spy(obj, &GlobalStatus::viewFlickingChanged);

    // Act：true（B1 变更）→ true（B2 同值）→ false（B1 复位）
    obj->setViewFlicking(true);
    const bool afterChange = obj->viewFlicking();
    obj->setViewFlicking(true);
    const int signalsAfterSameValue = spy.count();
    obj->setViewFlicking(false);
    const bool afterRevert = obj->viewFlicking();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, true);
    EXPECT_EQ(afterRevert, false);
}

TEST_F(GlobalStatusTest, SetEditMode_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 false）
    QSignalSpy spy(obj, &GlobalStatus::editModeChanged);

    // Act：true（B1 变更）→ true（B2 同值）→ false（B1 复位）
    obj->setEditMode(true);
    const bool afterChange = obj->editMode();
    obj->setEditMode(true);
    const int signalsAfterSameValue = spy.count();
    obj->setEditMode(false);
    const bool afterRevert = obj->editMode();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, true);
    EXPECT_EQ(afterRevert, false);
}

TEST_F(GlobalStatusTest, SetEditModified_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 false）
    QSignalSpy spy(obj, &GlobalStatus::editModifiedChanged);

    // Act：true（B1 变更）→ true（B2 同值）→ false（B1 复位）
    obj->setEditModified(true);
    const bool afterChange = obj->editModified();
    obj->setEditModified(true);
    const int signalsAfterSameValue = spy.count();
    obj->setEditModified(false);
    const bool afterRevert = obj->editModified();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, true);
    EXPECT_EQ(afterRevert, false);
}

TEST_F(GlobalStatusTest, SetAnimationBlock_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 false）
    QSignalSpy spy(obj, &GlobalStatus::animationBlockChanged);

    // Act：true（B1 变更）→ true（B2 同值）→ false（B1 复位）
    obj->setAnimationBlock(true);
    const bool afterChange = obj->animationBlock();
    obj->setAnimationBlock(true);
    const int signalsAfterSameValue = spy.count();
    obj->setAnimationBlock(false);
    const bool afterRevert = obj->animationBlock();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, true);
    EXPECT_EQ(afterRevert, false);
}

TEST_F(GlobalStatusTest, SetFullScreenAnimating_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 false）
    QSignalSpy spy(obj, &GlobalStatus::fullScreenAnimatingChanged);

    // Act：true（B1 变更）→ true（B2 同值）→ false（B1 复位）
    obj->setFullScreenAnimating(true);
    const bool afterChange = obj->fullScreenAnimating();
    obj->setFullScreenAnimating(true);
    const int signalsAfterSameValue = spy.count();
    obj->setFullScreenAnimating(false);
    const bool afterRevert = obj->fullScreenAnimating();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, true);
    EXPECT_EQ(afterRevert, false);
}

TEST_F(GlobalStatusTest, SetDelayInit_ChangeSameAndRevert_EmitsSignalOnlyOnChange)
{
    // Arrange（默认 true）
    QSignalSpy spy(obj, &GlobalStatus::delayInitChanged);

    // Act：false（B1 变更）→ false（B2 同值）→ true（B1 复位）
    obj->setDelayInit(false);
    const bool afterChange = obj->delayInit();
    obj->setDelayInit(false);
    const int signalsAfterSameValue = spy.count();
    obj->setDelayInit(true);
    const bool afterRevert = obj->delayInit();

    // Assert：两次变更各发一次信号，同值不发；getter 跟随最终值
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(afterChange, false);
    EXPECT_EQ(afterRevert, true);
}

// ── 同构参数化回归（bool 属性 ×11 / 宽度边界 ×4 / 页面 ×3）──

TEST_P(GlobalStatusBoolPropTest, SetBoolStateProperty_ToggleCycle_EmitsSignalOnlyOnChange)
{
    const BoolPropertyCase &c = GetParam();

    // Arrange（SetUp 新建实例，属性初值 = c.defaultValue；编译期直连分发）
    std::unique_ptr<QSignalSpy> spy = makeBoolSpy(obj, c.id);
    ASSERT_NE(spy, nullptr);

    // Act：翻转（B1）→ 同值重设（B2）→ 复位（B1）
    setBoolState(obj, c.id, !c.defaultValue);
    const bool afterToggle = boolState(obj, c.id);
    setBoolState(obj, c.id, !c.defaultValue);
    const int signalsAfterSameValue = spy->count();
    setBoolState(obj, c.id, c.defaultValue);
    const bool afterRestore = boolState(obj, c.id);

    // Assert：变更两次共 2 个信号，同值 0 个；getter 精确跟随
    EXPECT_EQ(afterToggle, !c.defaultValue);
    EXPECT_EQ(signalsAfterSameValue, 1);
    EXPECT_EQ(spy->count(), 2);
    EXPECT_EQ(afterRestore, c.defaultValue);
}

INSTANTIATE_TEST_SUITE_P(BoolStateProperties, GlobalStatusBoolPropTest,
                         ::testing::ValuesIn(boolPropertyCases()));

TEST_P(GlobalStatusThumbWidthTest, SetThumbnailVaildWidth_BoundaryValues_EmitsOnlyOnChange)
{
    const int width = GetParam();

    // Arrange（默认 storethumbnailVaildWidth = 0）
    QSignalSpy spy(obj, &GlobalStatus::thumbnailVaildWidthChanged);

    // Act：写入（B1 或 B2-当 width==0）→ 同值重写（B2）
    obj->setThumbnailVaildWidth(width);
    const int afterFirst = obj->thumbnailVaildWidth();
    obj->setThumbnailVaildWidth(width);
    const int afterSecond = obj->thumbnailVaildWidth();

    // Assert：getter 精确存取（含 0/负值/INT_MAX）；仅首次写入且值变化才发信号
    EXPECT_EQ(afterFirst, width);
    EXPECT_EQ(afterSecond, width);
    EXPECT_EQ(spy.count(), width != 0 ? 1 : 0);
}

INSTANTIATE_TEST_SUITE_P(ThumbnailWidthBoundary, GlobalStatusThumbWidthTest,
                         ::testing::Values(0, 360, -1, INT_MAX));

TEST_P(GlobalStatusStackPageTest, SetStackPage_PageTransitions_EmitsOnlyOnChange)
{
    const Types::StackPage page = GetParam();

    // Arrange（默认 storestackPage = Types::OpenImagePage）
    QSignalSpy spy(obj, &GlobalStatus::stackPageChanged);

    // Act：切页（B1，或 B2-当 page 为默认页）→ 同页重设（B2）
    obj->setStackPage(page);
    const Types::StackPage afterFirst = obj->stackPage();
    obj->setStackPage(page);
    const Types::StackPage afterSecond = obj->stackPage();

    // Assert：getter 精确存取；仅页面真正变化才发信号
    EXPECT_EQ(afterFirst, page);
    EXPECT_EQ(afterSecond, page);
    EXPECT_EQ(spy.count(), page != Types::OpenImagePage ? 1 : 0);
}

INSTANTIATE_TEST_SUITE_P(StackPageTransitions, GlobalStatusStackPageTest,
                         ::testing::Values(Types::OpenImagePage,
                                           Types::ImageViewPage,
                                           Types::SliderShowPage));
