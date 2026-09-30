# AT 用例覆盖率报告

日期：2026-09-20

## 统计范围

| 项目 | 路径 | 说明 |
|---|---|---|
| 原始用例 | `tests/at/cases_raw.yaml` | 共 140 条 |
| 当前 AT suite | `tests/at/yaml/**/*.suite.yaml` | 共 31 个 suite / 121 条 case |
| 待补清单 | `tests/at/case_backlog.md` | 当前无可自动化待补用例 |

## 覆盖率结论

| 指标 | 数量 |
|---|---:|
| 原始用例总数 | 140 |
| 已过滤用例数 | 68 |
| 可自动化原始用例数 | 72 |
| 已覆盖原始用例数 | 72 |
| 待补原始用例数 | 0 |
| 可自动化口径覆盖率 | 100.00% |

计算公式：`已覆盖原始用例数 / (原始用例总数 - 已过滤用例数)`。

当前 AT suite 最终运行结果：

```text
Suites: 31 passed, 0 failed, 0 error (31 total)
Specs:  121 passed, 0 failed, 0 skipped
```

## 过滤统计

| 过滤理由 | 数量 |
|---|---:|
| 第三方应用/跨应用集成 | 23 |
| 视觉/文案/布局/动画检查 | 18 |
| 包管理/命令行场景 | 6 |
| 性能/长时间/高频操作 | 6 |
| 外部设备/压缩包/网络挂载 | 4 |
| 窗口管理 | 4 |
| 第三方应用/系统设置依赖 | 4 |
| 系统级电源事件 | 2 |
| 触摸/压力/硬件输入 | 1 |

过滤理由总结：非标准GUI、视觉断言、系统及外部依赖。

## 非 1:1 覆盖说明

覆盖统计按原始用例 ID 累计。一个 suite case 覆盖多个原始用例时，按被覆盖的原始用例条数累计。

| suite case | 覆盖原始用例 | 说明 |
|---|---|---|
| `suite_jianshujiaohu_*` | `case_1652677`、`case_1652675`、`case_1652673`、`case_1652671`、`case_1652669`、`case_1652663`、`case_1652661`、`case_1652659`、`case_1652657`、`case_1652655`、`case_1652653`、`case_1652651` | 一组键鼠 suite 覆盖多个快捷键原始用例 |
| `suite_yjbuancaidan_*` | `case_1652143`、`case_1652217`、`case_1652221`、`case_1652237` 等 | 单个右键菜单主题 suite 覆盖多个原始右键菜单功能点 |
| `suite_wenguandaakaibuchong_001_s1`、`suite_wenguandaakaibuchong_002_s1` | `case_1652389`、`case_1652391`、`case_1652773` | 桌面/文管打开 suite 同时覆盖加载图片与格式打开能力 |

## 已覆盖原始用例清单

| 原始用例 ID | 原始用例标题 | 覆盖 suite case |
|---|---|---|
| `case_1652779` | [321][acp2][看图]视图切换、放大、旋转、翻转 | `suite_jianshujiaohu_007_s1`, `suite_jianshujiaohu_008_s1`, `suite_suofangchangjing_005_s1`, `suite_xuanzhuan_002_s1`, `suite_gongjulan_004_s1`, `suite_gongjulan_005_s1` |
| `case_1652773` | [320][acp2][看图]格式支持 | `suite_wenguandaakaibuchong_001_s1`, `suite_wenguandaakaibuchong_002_s1` |
| `case_1652745` | [071][smoke][acp1]图片格式-支持旋转格式图片检查 | `suite_xuanzhuan_002_s1`, `suite_xuanzhuan_003_s1` |
| `case_1652741` | [296][smoke][acp1]图片格式-支持设置壁纸格式图片检查 | `suite_yjbuancaidan_009_s1`, `suite_jianshujiaohu_003_s1` |
| `case_1652747` | [core]图片格式-右键菜单检查 | `case_1652747_s1` |
| `case_1652677` | [024][smoke][acp1]键鼠交互-快捷键Dow | `suite_jianshujiaohu_001_s1` |
| `case_1652675` | [153][smoke][acp1]键鼠交互-快捷键Ctrl+“-” | `suite_jianshujiaohu_002_s1`, `suite_suofangchangjing_002_s1` |
| `case_1652673` | [120][smoke][acp1]键鼠交互-快捷键Ctrl+F9 | `suite_jianshujiaohu_003_s1` |
| `case_1652671` | [149][smoke][acp1]键鼠交互-快捷键Right | `suite_jianshujiaohu_008_s1` |
| `case_1652669` | [150][smoke][acp1]键鼠交互-快捷键Left | `suite_jianshujiaohu_007_s1` |
| `case_1652667` | [101][smoke][acp1]键鼠交互-快捷键F11 | `case_1652667_s1`, `suite_quanping_002_s1` |
| `case_1652663` | [287][smoke][acp1]键鼠交互-快捷键鼠标滚轮 | `suite_jianshujiaohu_004_s1`, `suite_suofangchangjing_005_s1` |
| `case_1652661` | [050][smoke][acp1]键鼠交互-快捷键F2 | `suite_jianshujiaohu_010_s1`, `suite_chongmingming_001_s1` |
| `case_1652659` | [151][smoke][acp1]键鼠交互-快捷键Ctrl+I | `suite_jianshujiaohu_012_s1`, `suite_tupianxinxi_001_s1` |
| `case_1652657` | [121][smoke][acp1]键鼠交互-快捷键F1 | `suite_jianshujiaohu_005_s1`, `suite_zhucaidan_001_s1` |
| `case_1652655` | [152][smoke][acp1]键鼠交互-快捷键Ctrl+C | `suite_jianshujiaohu_011_s1`, `suite_fuzhi_002_s1` |
| `case_1652653` | [023][smoke][acp1]键鼠交互-快捷键Up | `suite_jianshujiaohu_006_s1` |
| `case_1652651` | [154][smoke][acp1]键鼠交互-快捷键Ctrl+“+” | `suite_jianshujiaohu_009_s1`, `suite_suofangchangjing_001_s1` |
| `case_1652649` | [118][smoke][acp1]键鼠交互-快捷键Alt+D | `suite_jianshujiaohubuchong_001_s1` |
| `case_1652647` | [119][smoke][acp1]键鼠交互-快捷键Ctrl+O | `suite_jianshujiaohubuchong_002_s1` |
| `case_1652645` | [103][smoke][acp1]键鼠交互-快捷键F5 | `case_1652645_s1`, `suite_huanjidengpian_001_s1` |
| `case_1652643` | [117][smoke][acp1]键鼠交互-快捷键Ctrl+Shift+/ | `case_1652643_s1`, `suite_jianshujiaohubuchong_003_s1` |
| `case_1652391` | 加载图片-文管中双击图片 | `suite_wenguandaakaibuchong_002_s1` |
| `case_1652389` | 加载图片-桌面双击打开图片 | `suite_wenguandaakaibuchong_001_s1` |
| `case_2003949` | 主菜单-关于 | `case_2003949_s1` |
| `case_1652347` | 主菜单-帮助 | `suite_zhucaidan_001_s1` |
| `case_1652343` | [210]重命名-文件名字符类型检查 | `case_1652343_s1` |
| `case_1652307` | [350]重命名-修改名称与其他非同格式图片重名时，检查重命名窗口展示 | `case_1652307_s1` |
| `case_1652285` | [212]重命名-文件名字符长度检查 | `case_1652285_s1` |
| `case_1652283` | [065][smoke][acp1]重命名-只读图片重命名 | `case_1652283_s1` |
| `case_1652281` | [003][smoke][acp1]重命名-读写图片重命名 | `suite_chongmingming_002_s1`, `suite_chongmingming_001_s1` |
| `case_1652255` | [214][smoke][acp1]旋转-全屏快捷键顺时针旋转 | `suite_xuanzhuan_001_s1` |
| `case_1652245` | [216][core][acp1]旋转-全屏右键菜单逆时针旋转 | `suite_xuanzhuan_003_s1` |
| `case_1652239` | [224][core][acp1]文管打开-全屏下右键菜单在文管中显示 | `suite_yjbuancaidan_010_s1` |
| `case_1652237` | [022][smoke][acp1]文管打开-窗口下右键菜单在文管中显示 | `suite_yjbuancaidan_010_s1` |
| `case_1652225` | [042][smoke][acp1]图片信息-工具栏切换图片展示 | `suite_tupianxinxi_001_s1`, `suite_gongjulan_001_s1`, `suite_gongjulan_002_s1` |
| `case_1652221` | 图片信息-右键菜单打开图片信息 | `suite_tupianxinxi_002_s1`, `suite_yjbuancaidan_007_s1` |
| `case_1652215` | [073][smoke][acp1]壁纸-窗口下不能设置壁纸 | `case_1652215_s1` |
| `case_1652217` | 壁纸-窗口下设置壁纸 | `suite_yjbuancaidan_009_s1` |
| `case_1652211` | 删除-删除单张图片后看图界面展示 | `case_1652211_s1` |
| `case_1652213` | 删除-右键菜单删除 | `suite_yjbuancaidan_003_s1` |
| `case_1652207` | 全屏-双击图片全屏 | `case_1652207_s1`, `suite_quanping_001_s1` |
| `case_1652203` | [046][smoke][acp1]全屏-快捷键退出全屏 | `case_1652203_s1`, `suite_quanping_002_s1` |
| `case_1652199` | 全屏-右键菜单退出全屏 | `case_1652199_s1` |
| `case_1652197` | 幻灯片-ESC退出幻灯片放映 | `suite_quanping_003_s1`, `suite_huanjidengpian_001_s1` |
| `case_1652159` | 幻灯片-多张图片循环放映 | `suite_quanping_004_s1`, `suite_huanjidengpian_002_s1` |
| `case_1652149` | 复制-复制大分辨率图片 | `case_1652149_s1` |
| `case_1652143` | [021][acp1][smoke]打印预览-图片右键唤起打印 | `suite_dayin_001_s1`, `suite_yjbuancaidan_006_s1` |
| `case_1652139` | [057][acp1][smoke]缩放-最小缩小倍数检查 | `suite_suofangchangjing_002_s1`, `suite_jianshujiaohu_001_s1` |
| `case_1652137` | [058][acp1][smoke]缩放-最大放大倍数检查 | `suite_suofangchangjing_001_s1`, `suite_jianshujiaohu_006_s1` |
| `case_1652135` | [002][acp1][smoke]工具栏-下一张按钮 | `case_1652135_s1`, `suite_gongjulan_001_s1` |
| `case_1652133` | [006\\061重复][acp1][smoke]工具栏-上一张按钮 | `case_1652133_s1`, `suite_gongjulan_002_s1` |
| `case_1652119` | [256][smoke]键鼠切换-鼠标滚动缩放图片过程中按下ctrl键 | `case_1652119_s1` |
| `case_1652095` | [295][smoke]键鼠切换-光标在预览区域图片上时检查图片切换功能 | `case_1652095_s1` |
| `case_1652089` | [294][smoke]键鼠切换-光标在预览区域，图片旁的空白区域上时检查图片切换功能 | `case_1652089_s1` |
| `case_1652081` | 键鼠切换-光标在工具栏缩略图区域时检查图片切换功能 | `case_1652081_s1` |
| `case_1652079` | 键鼠切换-光标在工具栏非缩略图区域时检查图片切换功能 | `case_1652079_s1` |
| `case_1652077` | 键鼠切换-光标在标题栏时检查图片切换功能 | `case_1652077_s1` |
| `case_1652073` | [259][core]键鼠切换-首张图时切换上一张 | `case_1652073_s1` |
| `case_1652071` | tif多页图-工具栏切换上一张图片 | `suite_tifduoyetu_001_s1` |
| `case_1652069` | tif多页图-切换页后检查缩略 | `suite_tifduoyetu_001_s1` |
| `case_1652067` | 键鼠切换-tif多页图检查图片切换功能 | `case_1652067_s1` |
| `case_1652061` | [038][acp1][core]删除-只读权限图片删除检查 | `case_1652061_s1` |
| `case_1652039` | tif多页图-缩略图切换图片 | `suite_tifduoyetu_001_s1` |
| `case_1651975` | [075][acp1][smoke]工具栏-全屏旋转按钮 | `case_1651975_s1` |
| `case_1651973` | [309][core]工具栏-横图适应窗口 | `case_1651973_s1` |
| `case_1651971` | [008][acp1][core]工具栏-竖图适应窗口 | `case_1651971_s1` |
| `case_1651939` | [206][acp1][smoke]导航窗口-按钮关闭导航窗口 | `case_1651939_s1` |
| `case_1651933` | [111][acp1][smoke]导航窗口-右键菜单唤起或关闭 | `case_1651933_s1` |
| `case_1651955` | 导航窗口-移动显示区域 | `case_1651955_s1` |
| `case_1651905` | tif多页图-播放幻灯片 | `suite_tifduoyetu_002_s1`, `suite_quanping_005_s1` |
| `case_1651809` | OCR-入口检查-窗口下点击工具栏按钮 | `suite_ocr_001_s1`, `suite_gongjulan_003_s1` |

## 待补概览

当前无可自动化待补原始用例，详见 `tests/at/case_backlog.md`。

| 模块 | 原始用例数 | 优先级 |
|---|---:|---|
| 全部可自动化模块 | 0 | 已补齐 |
