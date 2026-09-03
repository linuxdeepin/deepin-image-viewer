// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | PathViewRangeHandler(QObject*) | low | - | 1 | 1 |
// | ~PathViewRangeHandler | low | - | 1 | 2 |
// | enableForward() | low | - | 1 | 1 |
// | enableBackward() | low | - | 1 | 1 |
// | setEnableForward(bool) | low | - | 1 | 2 |
// | setEnableBackward(bool) | low | - | 1 | 2 |
// | target() | mid | - | 2 | 2 |
// | setTarget(QQuickItem*) | low | - | 1 | 4 |
// | eventFilter(QObject*, QEvent*) | high | - | 3 | 4 (TEST_F) + 7 (TEST_P 实例) = 11 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] （方向开关 × 位移方向 dx>0/dx<0/dx=0）
// 3. 每个等价类的边界值显式覆盖: [x] （dx=0 等值边界、target=nullptr、view==targetView）
// 4. 同质 ≥ 3 组用 TEST_P: [x] （7 组方向过滤参数）
// 5. 分支清单 → 用例映射已列出: [x] （见下方分支清单）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] （源码无 throw 分支，不适用）
// 8. 负面场景有专门用例: [x] （重复 setTarget/setEnable 同值、非目标对象事件、null target）
// 9. 负面用例验证强异常安全: [x] （同值设置后 flag/target/信号数不变）
// 10. stub_ext vs gMock 选择正确: [x] （纯逻辑类，无外部依赖，无需 stub/gMock）
//
// 分支清单（来源：get_code_snippet pathviewrangehandler.cpp eventFilter）
// B0: if (QEvent::MouseButtonRelease == event->type()) → basePoint 重置为空 QPointF
//     （位于 B1 提前放行之前，双向允许时 Release 也会重置，防止残留状态误过滤）
// B1: if (enableForwardFlag && enableBackwardFlag && obj == targetView) → 双向放行且来自目标视图
// B2: case QEvent::MouseMove → 进入鼠标移动处理（dynamic_cast 后按 basePoint 分流）
// B3: if (basePoint.isNull()) → basePoint = mouseEvent->position()（首次移动记录基准点）
// B4: if (!enableForwardFlag && newPoint.x() > basePoint.x()) → filter = true（禁止前移却前移）
// B5: if (!enableBackwardFlag && newPoint.x() < basePoint.x()) → filter = true（禁止后移却后移）
// B6: if (filter) → event->ignore()（过滤命中）
// B7: default → break → 落到末尾 return false（其它事件类型放行）
// B8: return false（B1 早退：双向允许 + obj == targetView，事件不处理）
// B9: return true（B6 过滤命中路径，事件被吞掉）
// 用例映射：
// - EventFilter_BothEnabledOnTarget_PassesThrough                      → B1/B8
// - EventFilter_MouseButtonRelease_ResetsBasePoint                     → B0（含 B1 早退前的重置）
// - EventFilter_MouseMoveOnNonTargetObject_SetsBasePoint               → B2+B3
// - EventFilter_MouseMoveDirection_FilteringMatchesFlags（TEST_P×7）   → B4/B5/B6/B9 过滤 + B1/B8 放行 + dx==0 边界放行
// - EventFilter_UnhandledEventType_PassesThrough                       → B7
//
// 分支清单（来源：get_code_snippet pathviewrangehandler.cpp:30-49 setTarget）
// B1: if (view == targetView) → 无任何操作、不发信号
// B2: if (旧 targetView 非空) → removeEventFilter(旧)
// B3: if (新 view 非空) → installEventFilter(新)
// B4: if (view != targetView)（含新 view 为 null）→ 置 targetView + Q_EMIT targetChanged()
// 用例映射：
// - SetTarget_SameViewTwice_DoesNotEmitTargetChanged                   → B1
// - SetTarget_NewView_InstallsFilterAndEmitsChanged                    → B3+B4
// - SetTarget_Retarget_RemovesFilterFromOldView                        → B2+B3+B4
// - SetTarget_NullAfterView_ClearsTargetAndEmits                       → B2+B4（新 view 为 null）
//
// 分支清单（来源：get_code_snippet pathviewrangehandler.cpp:57-65/73-81 setEnableForward/setEnableBackward）
// E1: b != flag → 置位 + Q_EMIT xxxChanged
// E2: b == flag → 不置位、不发信号
// 用例映射：
// - SetEnableForward_ChangedValue_EmitsSignalOnce / SetEnableBackward_ChangedValue_EmitsSignalOnce   → E1
// - SetEnableForward_SameValue_DoesNotEmitSignal / SetEnableBackward_SameValue_DoesNotEmitSignal     → E2

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointF>
#include <QQuickItem>
#include <QSignalSpy>

#include "stub_ext/stubext.h"
#include "pathviewrangehandler.h"

namespace {

// 鼠标移动方向过滤参数：两次 MouseMove（第二次相对 basePoint 偏移 dx）
struct MoveFilterCase {
    bool forwardEnabled;    // eventFilter 前置 setEnableForward
    bool backwardEnabled;   // eventFilter 前置 setEnableBackward
    qreal dx;               // 第二次移动的 x 偏移
    bool expectFiltered;    // 期望 eventFilter 返回 true 且事件被 ignore
};

// 经 volatile 汇阻断编译器去虚化/内联：delete 基类指针必须经 vtable 调用
// out-of-line 析构链（deleting dtor D0 → base dtor D2，符号发射于 pathviewrangehandler.cpp）。
// 注：GCC 禁止对 virtual 析构取成员函数指针（&T::~T 报错），故用此路径替代。
void deleteViaVtableDestruct(QObject *obj)
{
    QObject *volatile sink = obj;   // 存入 volatile 汇，切断指针来源追踪
    QObject *opaque = sink;         // 读回运行期值，静态类型仅 QObject*
    delete opaque;                  // 虚析构：vtable 派发 → D0 → D2 均为真实调用
}

}  // namespace

class PathViewRangeHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        obj = new PathViewRangeHandler();
    }

    void TearDown() override {
        delete obj;
        stub.clear();
    }

    stub_ext::StubExt stub;
    PathViewRangeHandler *obj = nullptr;
};

// ═══════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F 包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════

TEST_F(PathViewRangeHandlerTest, PathViewRangeHandler_WithParent_SetsParentAndDefaultState)
{
    // Arrange
    QObject parent;

    // Act
    PathViewRangeHandler *handler = new PathViewRangeHandler(&parent);

    // Assert
    EXPECT_EQ(handler->parent(), &parent);
    EXPECT_EQ(handler->target(), nullptr);
    EXPECT_TRUE(handler->enableForward());   // branch: enableForwardFlag { true }
    EXPECT_TRUE(handler->enableBackward());  // branch: enableBackwardFlag { true }
}

TEST_F(PathViewRangeHandlerTest, Destructor_DeleteHandler_EmitsDestroyedAndDetachesFromParent)
{
    // Arrange
    QObject parent;
    PathViewRangeHandler *handler = new PathViewRangeHandler(&parent);
    QSignalSpy destroyedSpy(handler, &QObject::destroyed);

    // Act
    delete handler;

    // Assert
    EXPECT_EQ(destroyedSpy.count(), 1);
    EXPECT_TRUE(parent.children().isEmpty());
}

TEST_F(PathViewRangeHandlerTest, Destructor_ViaOpaqueBasePointerDelete_RunsOutOfLineDtorChain)
{
    // Arrange：堆上构造（带父对象）；volatile 汇阻断去虚化后，
    // delete 基类指针经 vtable 强制执行 out-of-line 析构链（D0→D2）
    QObject parent;
    PathViewRangeHandler *handler = new PathViewRangeHandler(&parent);
    ASSERT_EQ(handler->parent(), &parent);
    EXPECT_EQ(handler->enableForward(), true);   // branch: 默认 enableForwardFlag = true（构造完成前置）
    bool destroyedEmitted = false;
    QObject::connect(handler, &QObject::destroyed,
                     [&destroyedEmitted](QObject *) { destroyedEmitted = true; });

    // Act：经不透明基类指针 delete，强制走 vtable 的 out-of-line 析构链
    deleteViaVtableDestruct(handler);

    // Assert：析构完整执行——destroyed 信号发出、从父对象 children 摘除、无崩溃
    EXPECT_TRUE(destroyedEmitted);
    EXPECT_TRUE(parent.children().isEmpty());
}

TEST_F(PathViewRangeHandlerTest, EnableForward_DefaultThenDisabled_ReturnsTrueThenFalse)
{
    // Arrange
    const bool defaultValue = obj->enableForward();

    // Act
    obj->setEnableForward(false);

    // Assert
    EXPECT_EQ(defaultValue, true);             // branch: 默认 enableForwardFlag = true
    EXPECT_EQ(obj->enableForward(), false);    // branch: setEnableForward(false) 置位
}

TEST_F(PathViewRangeHandlerTest, EnableBackward_DefaultThenDisabled_ReturnsTrueThenFalse)
{
    // Arrange
    const bool defaultValue = obj->enableBackward();

    // Act
    obj->setEnableBackward(false);

    // Assert
    EXPECT_EQ(defaultValue, true);              // branch: 默认 enableBackwardFlag = true
    EXPECT_EQ(obj->enableBackward(), false);    // branch: setEnableBackward(false) 置位
}

TEST_F(PathViewRangeHandlerTest, SetEnableForward_ChangedValue_EmitsSignalOnce)
{
    // Arrange
    QSignalSpy spy(obj, &PathViewRangeHandler::enableForwardChanged);

    // Act
    obj->setEnableForward(false);

    // Assert  // branch E1: b != flag → 置位 + 发信号
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(obj->enableForward());
}

TEST_F(PathViewRangeHandlerTest, SetEnableForward_SameValue_DoesNotEmitSignal)
{
    // Arrange
    QSignalSpy spy(obj, &PathViewRangeHandler::enableForwardChanged);

    // Act
    obj->setEnableForward(true);   // 与默认值相同

    // Assert  // branch E2: b == flag → 状态与信号数均不变（强异常安全）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(obj->enableForward());
}

TEST_F(PathViewRangeHandlerTest, SetEnableBackward_ChangedValue_EmitsSignalOnce)
{
    // Arrange
    QSignalSpy spy(obj, &PathViewRangeHandler::enableBackwardChanged);

    // Act
    obj->setEnableBackward(false);

    // Assert  // branch E1: b != flag → 置位 + 发信号
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(obj->enableBackward());
}

TEST_F(PathViewRangeHandlerTest, SetEnableBackward_SameValue_DoesNotEmitSignal)
{
    // Arrange
    QSignalSpy spy(obj, &PathViewRangeHandler::enableBackwardChanged);

    // Act
    obj->setEnableBackward(true);   // 与默认值相同

    // Assert  // branch E2: b == flag → 状态与信号数均不变（强异常安全）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(obj->enableBackward());
}

TEST_F(PathViewRangeHandlerTest, Target_ByDefault_ReturnsNullptr)
{
    // Arrange
    QSignalSpy spy(obj, &PathViewRangeHandler::targetChanged);

    // Act
    QQuickItem *defaultTarget = obj->target();
    obj->setTarget(nullptr);   // 负面输入：null 且与默认相同

    // Assert  // branch: targetView { nullptr } 且 setTarget B1: view == targetView → 无操作
    EXPECT_EQ(defaultTarget, nullptr);
    EXPECT_EQ(obj->target(), nullptr);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(PathViewRangeHandlerTest, Target_AfterSetTarget_ReturnsQuickItem)
{
    // Arrange
    QQuickItem view;
    QSignalSpy spy(obj, &PathViewRangeHandler::targetChanged);

    // Act
    obj->setTarget(&view);

    // Assert  // branch T4→B4: view != targetView → 置 targetView + 发信号
    EXPECT_EQ(obj->target(), &view);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(PathViewRangeHandlerTest, SetTarget_SameViewTwice_DoesNotEmitTargetChanged)
{
    // Arrange
    QQuickItem view;
    obj->setTarget(&view);
    QSignalSpy spy(obj, &PathViewRangeHandler::targetChanged);

    // Act
    obj->setTarget(&view);   // 重复设置同一 view

    // Assert  // branch T1→B1: view == targetView → 无任何操作（强异常安全）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->target(), &view);
}

TEST_F(PathViewRangeHandlerTest, SetTarget_NewView_InstallsFilterAndEmitsChanged)
{
    // Arrange
    // 关闭 forward 使 B1 提前返回失效，从而可观测过滤器副作用（basePoint 更新）
    obj->setEnableForward(false);
    QQuickItem view;
    QSignalSpy spy(obj, &PathViewRangeHandler::targetChanged);

    // Act
    obj->setTarget(&view);
    const QPointF pos(42, 24);
    QMouseEvent move(QEvent::MouseMove, pos, pos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&view, &move);   // 经真实事件派发验证 installEventFilter

    // Assert  // branch T3+T4→B3+B4: 新 view 非空 → 安装过滤器 + 发信号
    EXPECT_EQ(obj->target(), &view);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(obj->basePoint == pos);   // 过滤器已安装并被回调（-fno-access-control）
}

TEST_F(PathViewRangeHandlerTest, SetTarget_Retarget_RemovesFilterFromOldView)
{
    // Arrange
    obj->setEnableForward(false);
    QQuickItem oldView;
    QQuickItem newView;
    QSignalSpy spy(obj, &PathViewRangeHandler::targetChanged);
    obj->setTarget(&oldView);

    // Act
    obj->setTarget(&newView);
    const QPointF oldPos(10, 10);
    const QPointF newPos(99, 99);
    QMouseEvent moveOld(QEvent::MouseMove, oldPos, oldPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&oldView, &moveOld);   // 旧 view 事件不应再进入过滤器
    const bool basePointUntouchedByOldView = obj->basePoint.isNull();
    QMouseEvent moveNew(QEvent::MouseMove, newPos, newPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&newView, &moveNew);   // 新 view 事件应进入过滤器

    // Assert  // branch T2+T3→B2+B3: 旧 view 移除过滤器、新 view 安装过滤器
    EXPECT_EQ(obj->target(), &newView);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_TRUE(basePointUntouchedByOldView);    // 旧 view 未被过滤 → basePoint 未被旧事件写入
    EXPECT_TRUE(obj->basePoint == newPos);       // 新 view 已被过滤 → basePoint 记录新事件位置
}

TEST_F(PathViewRangeHandlerTest, SetTarget_NullAfterView_ClearsTargetAndEmits)
{
    // Arrange
    QQuickItem view;
    obj->setTarget(&view);
    QSignalSpy spy(obj, &PathViewRangeHandler::targetChanged);

    // Act
    obj->setTarget(nullptr);

    // Assert  // branch T2+T4→B2+B4: 旧 targetView 非空 → removeEventFilter；新 view 为 null → targetView = null + 发信号
    EXPECT_EQ(obj->target(), nullptr);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(PathViewRangeHandlerTest, EventFilter_BothEnabledOnTarget_PassesThrough)
{
    // Arrange
    QQuickItem view;
    obj->setTarget(&view);
    const QPointF pos(5, 6);
    QMouseEvent move(QEvent::MouseMove, pos, pos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);

    // Act
    const bool filtered = obj->eventFilter(&view, &move);

    // Assert  // branch B1/B9: 双向允许且 obj == targetView → 提前 return false（basePoint 不更新）
    EXPECT_EQ(filtered, false);
    EXPECT_EQ(obj->basePoint, QPointF());
    EXPECT_EQ(move.isAccepted(), true);
}

TEST_F(PathViewRangeHandlerTest, EventFilter_MouseButtonRelease_ResetsBasePoint)
{
    // Arrange
    obj->setEnableForward(false);   // 先使 B1 不成立，进入 switch 记录基准点
    QQuickItem view;
    obj->setTarget(&view);
    const QPointF base(30, 40);
    QMouseEvent move(QEvent::MouseMove, base, base, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    obj->eventFilter(&view, &move);   // 先建立 basePoint
    const bool basePointEstablished = !obj->basePoint.isNull();
    obj->setEnableForward(true);   // 恢复双向允许：Release 需在提前 return 前重置 basePoint
    QMouseEvent release(QEvent::MouseButtonRelease, base, base, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);

    // Act
    const bool filtered = obj->eventFilter(&view, &release);

    // Assert  // branch B0: MouseButtonRelease → basePoint 重置为空（双向允许下同样生效，防残留误过滤）
    EXPECT_EQ(basePointEstablished, true);   // 前置：MouseMove 已记录 basePoint
    EXPECT_EQ(filtered, false);
    EXPECT_EQ(obj->basePoint, QPointF());
}

TEST_F(PathViewRangeHandlerTest, EventFilter_MouseMoveOnNonTargetObject_SetsBasePoint)
{
    // Arrange
    QQuickItem view;
    obj->setTarget(&view);
    QObject stranger;   // 非 targetView → B1 因 obj 不匹配而不成立
    const QPointF pos(11, 22);
    QMouseEvent move(QEvent::MouseMove, pos, pos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);

    // Act
    const bool filtered = obj->eventFilter(&stranger, &move);

    // Assert  // branch B3+B4: MouseMove 且 basePoint 为空 → 记录鼠标位置，事件放行
    EXPECT_EQ(filtered, false);
    EXPECT_EQ(obj->basePoint, pos);
    EXPECT_EQ(move.isAccepted(), true);
}

TEST_F(PathViewRangeHandlerTest, EventFilter_UnhandledEventType_PassesThrough)
{
    // Arrange
    obj->setEnableForward(false);   // 使 B1 不成立，进入 switch
    QQuickItem view;
    obj->setTarget(&view);
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);

    // Act
    const bool filtered = obj->eventFilter(&view, &keyPress);

    // Assert  // branch B8: default → 事件放行且状态不变
    EXPECT_EQ(filtered, false);
    EXPECT_EQ(keyPress.isAccepted(), true);
    EXPECT_EQ(obj->basePoint, QPointF());
}

// ─── 方向过滤参数化（同质多组输入：开关组合 × 位移方向） ───

class PathViewRangeHandlerMoveTest : public ::testing::TestWithParam<MoveFilterCase> {
protected:
    void SetUp() override {
        stub.clear();
        handler = new PathViewRangeHandler();
        view = new QQuickItem();
        handler->setTarget(view);
        handler->setEnableForward(GetParam().forwardEnabled);
        handler->setEnableBackward(GetParam().backwardEnabled);
    }

    void TearDown() override {
        delete handler;
        delete view;
        stub.clear();
    }

    stub_ext::StubExt stub;
    PathViewRangeHandler *handler = nullptr;
    QQuickItem *view = nullptr;
};

TEST_P(PathViewRangeHandlerMoveTest, EventFilter_MouseMoveDirection_FilteringMatchesFlags)
{
    const auto &c = GetParam();
    // Arrange: 第一次移动建立 basePoint
    const QPointF base(100.0, 50.0);
    QMouseEvent firstMove(QEvent::MouseMove, base, base, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    handler->eventFilter(view, &firstMove);

    // Act: 第二次移动沿 x 方向偏移 dx
    QMouseEvent secondMove(QEvent::MouseMove, base + QPointF(c.dx, 0), base + QPointF(c.dx, 0),
                           Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    const bool filtered = handler->eventFilter(view, &secondMove);

    // Assert  // B5/B6/B7/B10: 受阻方向过滤并 ignore；允许方向（B8 末尾）放行
    EXPECT_EQ(filtered, c.expectFiltered);
    EXPECT_EQ(secondMove.isAccepted(), !c.expectFiltered);
}

INSTANTIATE_TEST_SUITE_P(
    DirectionCases,
    PathViewRangeHandlerMoveTest,
    ::testing::Values(
        MoveFilterCase{true,  true,  30.0, false},   // B1: 双向允许 + obj==target → 提前放行
        MoveFilterCase{false, true,  30.0, true},    // B4+B6: 禁止前移且 dx>0 → 过滤
        MoveFilterCase{true,  false, -30.0, true},   // B5+B6: 禁止后移且 dx<0 → 过滤
        MoveFilterCase{false, true,  -30.0, false},  // B7: 禁止前移但 dx<0（允许方向）→ 放行
        MoveFilterCase{true,  false, 30.0, false},   // B7: 禁止后移但 dx>0（允许方向）→ 放行
        MoveFilterCase{false, true,   0.0,  false},  // B7 边界: dx==0 既不大于也不小于 → 放行
        MoveFilterCase{false, false,  5.0,  true}    // B4+B6: 双向禁止 → 任何非零位移均过滤
        ));
