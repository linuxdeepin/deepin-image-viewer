# AT 待补用例清单

日期：2026-09-20

## 统计摘要

| 指标 | 数量 |
|---|---:|
| 原始用例总数 | 140 |
| 已过滤用例数 | 68 |
| 可自动化原始用例数 | 72 |
| 已覆盖原始用例数 | 72 |
| 待补原始用例数 | 0 |
| 可自动化口径覆盖率 | 100.00% |

过滤理由总结：非标准GUI、视觉断言、系统及外部依赖。

## 当前结论

当前 `tests/at/yaml/**/*.suite.yaml` 已覆盖过滤后全部 72 条可自动化原始用例。

本轮补齐新增 `tests/at/yaml/待补覆盖/待补覆盖.suite.yaml`，共 20 条 case。

## 本轮补齐清单

| 原始用例 ID | 模块 | 覆盖 suite case | 优先级 |
|---|---|---|---|
| `case_2003949` | 主菜单 | `case_2003949_s1` | P1 |
| `case_1652747` | 图片格式 | `case_1652747_s1` | P1 |
| `case_1652343` | 重命名 | `case_1652343_s1` | P0 |
| `case_1652307` | 重命名 | `case_1652307_s1` | P0 |
| `case_1652285` | 重命名 | `case_1652285_s1` | P0 |
| `case_1652283` | 重命名 | `case_1652283_s1` | P0 |
| `case_1652215` | 壁纸 | `case_1652215_s1` | P1 |
| `case_1652211` | 删除 | `case_1652211_s1` | P0 |
| `case_1652199` | 全屏 | `case_1652199_s1` | P1 |
| `case_1652149` | 复制 | `case_1652149_s1` | P2 |
| `case_1652119` | 键鼠切换 | `case_1652119_s1` | P0 |
| `case_1652095` | 键鼠切换 | `case_1652095_s1` | P1 |
| `case_1652089` | 键鼠切换 | `case_1652089_s1` | P1 |
| `case_1652081` | 键鼠切换 | `case_1652081_s1` | P1 |
| `case_1652079` | 键鼠切换 | `case_1652079_s1` | P1 |
| `case_1652077` | 键鼠切换 | `case_1652077_s1` | P1 |
| `case_1652073` | 键鼠切换 | `case_1652073_s1` | P0 |
| `case_1652067` | tif 多页图 | `case_1652067_s1` | P1 |
| `case_1652061` | 删除 | `case_1652061_s1` | P0 |
| `case_1651955` | 导航窗口 | `case_1651955_s1` | P2 |

## 验证结果

新增补齐 suite 验证：

```text
Suites: 1 passed, 0 failed, 0 error (1 total)
Specs:  20 passed, 0 failed, 0 skipped
```

最终全量回归：

```text
Suites: 31 passed, 0 failed, 0 error (31 total)
Specs:  121 passed, 0 failed, 0 skipped
```

日志：`/tmp/opencode/final_after_backlog.log`
