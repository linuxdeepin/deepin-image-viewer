// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private 1.0 as P

// 工具栏图标按钮：使用 CrystalColor 家族 + 自定义 D.Palette 覆盖背景色，
// 在浅色主题下呈现半透明白色（而非 CrystalColor 默认的半透明黑色），
// 使按钮能透出 FloatingPanel 容器提供的背景模糊效果。
// 覆盖模式参考 DTK 原生 ButtonBox.qml：在 P.ButtonPanel 上直接设置 color1/color2。
D.IconButton {
    id: control

    D.ColorSelector.family: D.Palette.CrystalColor

    background: P.ButtonPanel {
        button: control
        implicitWidth: DS.Style.iconButton.backgroundSize
        implicitHeight: DS.Style.iconButton.backgroundSize

        // 覆盖内层描边色：DTK 默认 insideBorder crystal 在 hover 时从白切换到黑(rgba(0,0,0,0.05))，
        // 叠加在已变暗的背景上产生"凹槽"感，使边缘显得模糊。这里强制 hover/pressed 时内层保持白色，
        // 维持"外黑内白"的清晰双层描边结构。normal 保持 DTK 默认值不变。
        insideBorderColor: D.Palette {
            normal {
                common: Qt.rgba(1, 1, 1, 0.1)
                crystal: Qt.rgba(1, 1, 1, 0.1)
            }
            normalDark {
                common: Qt.rgba(1, 1, 1, 0.1)
                crystal: Qt.rgba(1, 1, 1, 0.1)
            }
            hovered {
                common: Qt.rgba(1, 1, 1, 0.2)
                crystal: Qt.rgba(1, 1, 1, 0.15)
            }
            pressed {
                common: Qt.rgba(1, 1, 1, 0.03)
                crystal: Qt.rgba(0, 0, 0, 0.03)
            }
        }

        // 覆盖外层描边色：DTK 源码 hovered 用简写（仅 common），crystal 回退到 normal 的 8% 黑，
        // 导致 hover 时外边框与 normal 完全一样。这里为 crystal 补上真正生效的 hover 值 20% 黑。
        // normal/pressed 保持 DTK 默认值不变。
        outsideBorderColor: D.Palette {
            normal {
                common: Qt.rgba(0, 0, 0, 0.08)
                crystal: Qt.rgba(0, 0, 0, 0.08)
            }
            hovered {
                common: Qt.rgba(0, 0, 0, 0.2)
                crystal: Qt.rgba(0, 0, 0, 0.2)
            }
            pressed {
                common: ("transparent")
                crystal: ("transparent")
            }
        }

        // 覆盖默认的 DS.Style.button.background1/2。
        // 浅色主题：normal 40% 白（玻璃质感，增强可见度），hovered 10% 黑（变暗反馈），pressed 15% 黑（加深）。
        // 深色主题：保持 DTK 原生 crystal 默认值不变（normal 8% 白，hovered 20% 白，pressed 5% 白）。
        color1: D.Palette {
            normal {
                common: Qt.rgba(0, 0, 0, 0.1)
                crystal: Qt.rgba(1, 1, 1, 0.4)
            }
            normalDark {
                common: Qt.rgba(1, 1, 1, 0.1)
                crystal: Qt.rgba(1, 1, 1, 0.08)
            }
            hovered {
                common: Qt.rgba(0, 0, 0, 0.2)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            hoveredDark {
                common: Qt.rgba(1, 1, 1, 0.2)
                crystal: Qt.rgba(1, 1, 1, 0.2)
            }
            pressed {
                common: Qt.rgba(0, 0, 0, 0.15)
                crystal: Qt.rgba(0, 0, 0, 0.15)
            }
            pressedDark {
                common: Qt.rgba(1, 1, 1, 0.05)
                crystal: Qt.rgba(1, 1, 1, 0.05)
            }
        }
        color2: color1
    }
}
