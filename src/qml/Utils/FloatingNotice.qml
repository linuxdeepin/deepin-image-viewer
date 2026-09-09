// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0

//浮动提示框定义
//uos-design: 瞬态应用内提示必须交由 D.FloatingMessage / MessageManager 承载（勿自绘外壳）。
//实际显示、主题配色、出入场动画与自动关闭均由 D.FloatingMessage 标准面板完成。
Item {
    id: root

    // 提示存续时长（毫秒），与旧实现 Timer.interval=1500 保持一致
    readonly property int duration: 1500
    // 消息标识：相同 msgId 的消息会被 MessageManager 去重并就地更新内容
    readonly property string msgId: "FloatLabel"

    // content: 要展示的文本，如 "200%"
    function show(content) {
        DTK.sendMessage(root, content, "", duration, msgId);
    }

    // 立即关闭当前消息（无出场动画，对应旧实现 visible=false 的即时消失）
    function close() {
        DTK.closeMessage(root, msgId);
    }
}
