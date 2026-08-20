// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV

Item {
    id: editCanvas

    Accessible.name: "EditCanvas" + strokes.length
    Accessible.role: Accessible.Pane

    property color currentColor: "#e53935"
    property string currentTool: "pen"
    property int effectStrength: 15
    property int selectedIndex: -1
    property var strokes: []
    property string blurMode: "gaussian"
    property rect cropRect: Qt.rect(0, 0, 0, 0)
    property real previousCanvasHeight: 0
    property real previousCanvasWidth: 0
    property string textMode: "plain"
    property int thickness: 2
    property var vectorHistory: []
    property int vectorHistoryIndex: -1

    signal strokeCommitted
    signal effectRequested(string effect, rect normalizedRect, int strength)
    signal cropRequested(rect normalizedRect)

    function clear() {
        strokes = [];
        selectedIndex = -1;
        drawingCanvas.requestPaint();
    }

    function initializeHistory() {
        vectorHistory = [[]];
        vectorHistoryIndex = 0;
    }

    function cloneStrokes(source, normalize) {
        var result = [];
        for (var i = 0; i < source.length; ++i) {
            var points = [];
            for (var p = 0; p < source[i].points.length; ++p) {
                var pointX = source[i].points[p].x;
                var pointY = source[i].points[p].y;
                points.push(normalize ? Qt.point(pointX / Math.max(1, width), pointY / Math.max(1, height))
                                      : Qt.point(pointX * width, pointY * height));
            }
            result.push(Object.assign({}, source[i], { points: points }));
        }
        return result;
    }

    function commitHistory() {
        if (vectorHistoryIndex < 0) initializeHistory();
        var updated = vectorHistory.slice(0, vectorHistoryIndex + 1);
        updated.push(cloneStrokes(strokes, true));
        while (updated.length > 50) {
            updated.shift();
        }
        vectorHistory = updated;
        vectorHistoryIndex = updated.length - 1;
    }

    function undoHistory() {
        if (vectorHistoryIndex <= 0) return false;
        --vectorHistoryIndex;
        strokes = cloneStrokes(vectorHistory[vectorHistoryIndex], false);
        selectedIndex = -1;
        drawingCanvas.requestPaint();
        return true;
    }

    function redoHistory() {
        if (vectorHistoryIndex < 0 || vectorHistoryIndex + 1 >= vectorHistory.length) return false;
        ++vectorHistoryIndex;
        strokes = cloneStrokes(vectorHistory[vectorHistoryIndex], false);
        selectedIndex = -1;
        drawingCanvas.requestPaint();
        return true;
    }

    function boundsFor(stroke) {
        if (!stroke || stroke.points.length === 0) return null;
        var left = stroke.points[0].x;
        var right = left;
        var top = stroke.points[0].y;
        var bottom = top;
        for (var i = 1; i < stroke.points.length; ++i) {
            left = Math.min(left, stroke.points[i].x);
            right = Math.max(right, stroke.points[i].x);
            top = Math.min(top, stroke.points[i].y);
            bottom = Math.max(bottom, stroke.points[i].y);
        }
        return { left: left, right: right, top: top, bottom: bottom,
                 width: right - left, height: bottom - top };
    }

    function handleAt(x, y) {
        if (selectedIndex < 0 || selectedIndex >= strokes.length) return "";
        var bounds = boundsFor(strokes[selectedIndex]);
        var handles = [
            { name: "top", x: (bounds.left + bounds.right) / 2, y: bounds.top },
            { name: "right", x: bounds.right, y: (bounds.top + bounds.bottom) / 2 },
            { name: "bottom", x: (bounds.left + bounds.right) / 2, y: bounds.bottom },
            { name: "left", x: bounds.left, y: (bounds.top + bounds.bottom) / 2 },
            { name: "topLeft", x: bounds.left, y: bounds.top },
            { name: "topRight", x: bounds.right, y: bounds.top },
            { name: "bottomLeft", x: bounds.left, y: bounds.bottom },
            { name: "bottomRight", x: bounds.right, y: bounds.bottom }
        ];
        for (var i = 0; i < handles.length; ++i) {
            if (Math.abs(x - handles[i].x) <= 7 && Math.abs(y - handles[i].y) <= 7)
                return handles[i].name;
        }
        return "";
    }

    function strokeAt(x, y) {
        for (var i = strokes.length - 1; i >= 0; --i) {
            var stroke = strokes[i];
            var bounds = boundsFor(stroke);
            var tolerance = Math.max(6, stroke.width / 2);
            if (stroke.type === "text" || stroke.type === "number") {
                if (x >= bounds.left - tolerance && x <= bounds.right + tolerance
                        && y >= bounds.top - tolerance && y <= bounds.bottom + tolerance)
                    return i;
                continue;
            }
            if (stroke.type === "rect") {
                var insideX = x >= bounds.left - tolerance && x <= bounds.right + tolerance;
                var insideY = y >= bounds.top - tolerance && y <= bounds.bottom + tolerance;
                var nearVertical = Math.abs(x - bounds.left) <= tolerance
                        || Math.abs(x - bounds.right) <= tolerance;
                var nearHorizontal = Math.abs(y - bounds.top) <= tolerance
                        || Math.abs(y - bounds.bottom) <= tolerance;
                if ((insideY && nearVertical) || (insideX && nearHorizontal)) return i;
                continue;
            }
            if (stroke.type === "ellipse") {
                var radiusX = Math.max(bounds.width / 2, 0.5);
                var radiusY = Math.max(bounds.height / 2, 0.5);
                var normalizedX = (x - (bounds.left + bounds.right) / 2) / radiusX;
                var normalizedY = (y - (bounds.top + bounds.bottom) / 2) / radiusY;
                var distance = Math.sqrt(normalizedX * normalizedX + normalizedY * normalizedY);
                var ellipseTolerance = tolerance / Math.max(radiusX, radiusY);
                if (Math.abs(distance - 1) <= ellipseTolerance) return i;
                continue;
            }
            for (var p = 1; p < stroke.points.length; ++p) {
                var start = stroke.points[p - 1];
                var end = stroke.points[p];
                var deltaX = end.x - start.x;
                var deltaY = end.y - start.y;
                var lengthSquared = deltaX * deltaX + deltaY * deltaY;
                var ratio = lengthSquared === 0 ? 0
                                               : ((x - start.x) * deltaX + (y - start.y) * deltaY) / lengthSquared;
                ratio = Math.max(0, Math.min(1, ratio));
                var nearestX = start.x + ratio * deltaX;
                var nearestY = start.y + ratio * deltaY;
                var offsetX = x - nearestX;
                var offsetY = y - nearestY;
                if (offsetX * offsetX + offsetY * offsetY <= tolerance * tolerance) return i;
            }
        }
        return -1;
    }

    function replaceStroke(index, stroke) {
        var updated = strokes.slice();
        updated[index] = stroke;
        strokes = updated;
        drawingCanvas.requestPaint();
    }

    function removeSelected() {
        if (selectedIndex < 0 || selectedIndex >= strokes.length) return;
        var updated = strokes.slice();
        updated.splice(selectedIndex, 1);
        strokes = updated;
        selectedIndex = -1;
        strokeCommitted();
        drawingCanvas.requestPaint();
    }

    function rotateSelected() {
        if (selectedIndex < 0 || selectedIndex >= strokes.length) return;
        var stroke = strokes[selectedIndex];
        var bounds = boundsFor(stroke);
        var centerX = (bounds.left + bounds.right) / 2;
        var centerY = (bounds.top + bounds.bottom) / 2;
        var rotated = [];
        for (var i = 0; i < stroke.points.length; ++i) {
            var dx = stroke.points[i].x - centerX;
            var dy = stroke.points[i].y - centerY;
            rotated.push(Qt.point(centerX - dy, centerY + dx));
        }
        replaceStroke(selectedIndex, Object.assign({}, stroke, { points: rotated }));
        strokeCommitted();
    }

    function updateSelectedStyle(color, width) {
        if (selectedIndex < 0 || selectedIndex >= strokes.length) return;
        var stroke = Object.assign({}, strokes[selectedIndex],
                                   { color: color.toString(), width: width });
        replaceStroke(selectedIndex, stroke);
        strokeCommitted();
    }

    function nextNumber() {
        var used = {};
        for (var i = 0; i < strokes.length; ++i) {
            if (strokes[i].type !== "number") continue;
            var value = Number(strokes[i].number);
            if (Number.isInteger(value) && value > 0) used[value] = true;
        }
        var number = 1;
        while (used[number]) ++number;
        return number;
    }

    function addNumber(x, y) {
        var diameter = 30;
        var updated = strokes.slice();
        updated.push({
            type: "number",
            number: nextNumber(),
            points: [Qt.point(x - diameter / 2, y - diameter / 2),
                     Qt.point(x + diameter / 2, y + diameter / 2)],
            color: currentColor.toString(),
            fontFamily: textEditor.font.family,
            width: thickness,
            baseWidth: diameter,
            baseHeight: diameter
        });
        strokes = updated;
        selectedIndex = updated.length - 1;
        strokeCommitted();
        drawingCanvas.requestPaint();
    }

    function commitTextInput() {
        if (!textEditor.visible) return;
        var value = textEditor.text.trim();
        if (value.length > 0) {
            var textWidth = Math.max(20, textEditor.contentWidth);
            var textHeight = Math.max(24, textEditor.contentHeight);
            var updated = strokes.slice();
            updated.push({
                type: "text",
                text: value,
                points: [Qt.point(textEditor.x, textEditor.y),
                         Qt.point(textEditor.x + textWidth, textEditor.y + textHeight)],
                color: currentColor.toString(),
                fontFamily: textEditor.font.family,
                width: thickness,
                baseWidth: textWidth,
                baseHeight: textHeight
            });
            strokes = updated;
            selectedIndex = updated.length - 1;
            strokeCommitted();
        }
        textEditor.text = "";
        textEditor.visible = false;
        forceActiveFocus();
        drawingCanvas.requestPaint();
    }

    function repaint() {
        drawingCanvas.requestPaint();
    }

    function collectAnnotations() {
        var annotations = [];
        for (var i = 0; i < strokes.length; ++i) {
            var points = [];
            for (var p = 0; p < strokes[i].points.length; ++p) {
                points.push(Qt.point(strokes[i].points[p].x / Math.max(1, width),
                                     strokes[i].points[p].y / Math.max(1, height)));
            }
            annotations.push(Object.assign({}, strokes[i], {
                points: points,
                width: strokes[i].width / Math.max(1, width)
            }));
        }
        return annotations;
    }

    function resetCrop() {
        var marginX = width * 0.1;
        var marginY = height * 0.1;
        cropRect = Qt.rect(marginX, marginY, Math.max(2, width - marginX * 2),
                          Math.max(2, height - marginY * 2));
        drawingCanvas.requestPaint();
    }

    function cropHandleAt(x, y) {
        var handles = [
            { name: "topLeft", x: cropRect.x, y: cropRect.y },
            { name: "topRight", x: cropRect.x + cropRect.width, y: cropRect.y },
            { name: "bottomRight", x: cropRect.x + cropRect.width, y: cropRect.y + cropRect.height },
            { name: "bottomLeft", x: cropRect.x, y: cropRect.y + cropRect.height }
        ];
        for (var i = 0; i < handles.length; ++i) {
            if (Math.abs(x - handles[i].x) <= 8 && Math.abs(y - handles[i].y) <= 8)
                return handles[i].name;
        }
        if (x >= cropRect.x && x <= cropRect.x + cropRect.width
                && y >= cropRect.y && y <= cropRect.y + cropRect.height)
            return "move";
        return "";
    }

    function confirmCrop() {
        if (currentTool !== "crop" || cropRect.width < 2 || cropRect.height < 2) return;
        cropRequested(Qt.rect(cropRect.x / width, cropRect.y / height,
                              cropRect.width / width, cropRect.height / height));
    }

    function finishCrop() {
        var crop = cropRect;
        var updated = [];
        for (var i = 0; i < strokes.length; ++i) {
            var stroke = strokes[i];
            var bounds = boundsFor(stroke);
            if (bounds.right <= crop.x || bounds.left >= crop.x + crop.width
                    || bounds.bottom <= crop.y || bounds.top >= crop.y + crop.height)
                continue;
            var points = [];
            for (var p = 0; p < stroke.points.length; ++p) {
                points.push(Qt.point((stroke.points[p].x - crop.x) * width / crop.width,
                                     (stroke.points[p].y - crop.y) * height / crop.height));
            }
            updated.push(Object.assign({}, stroke, { points: points }));
        }
        strokes = updated;
        selectedIndex = -1;
        resetCrop();
        strokeCommitted();
        drawingCanvas.requestPaint();
    }

    function scaleStrokes(scaleX, scaleY) {
        if (strokes.length === 0 || !isFinite(scaleX) || !isFinite(scaleY)) return;
        var updated = [];
        for (var i = 0; i < strokes.length; ++i) {
            var points = [];
            for (var p = 0; p < strokes[i].points.length; ++p)
                points.push(Qt.point(strokes[i].points[p].x * scaleX, strokes[i].points[p].y * scaleY));
            updated.push(Object.assign({}, strokes[i], { points: points }));
        }
        strokes = updated;
    }

    onCurrentToolChanged: {
        if (currentTool === "crop") resetCrop();
    }
    onWidthChanged: {
        if (previousCanvasWidth > 0) scaleStrokes(width / previousCanvasWidth, 1);
        previousCanvasWidth = width;
        if (currentTool === "crop") resetCrop();
        drawingCanvas.requestPaint();
    }
    onHeightChanged: {
        if (previousCanvasHeight > 0) scaleStrokes(1, height / previousCanvasHeight);
        previousCanvasHeight = height;
        if (currentTool === "crop") resetCrop();
        drawingCanvas.requestPaint();
    }

    clip: true

    Canvas {
        id: drawingCanvas

        property var activePoints: []
        property string interaction: ""
        property bool interactionChanged: false
        property string resizeHandle: ""
        property point pressPoint: Qt.point(0, 0)
        property var originalPoints: []

        anchors.fill: parent
        renderStrategy: Canvas.Cooperative

        onPaint: {
            var context = getContext("2d");
            context.reset();
            context.clearRect(0, 0, width, height);
            context.lineCap = "round";
            context.lineJoin = "round";

            function drawStroke(stroke) {
                if (!stroke || stroke.points.length < 2) return;
                var start = stroke.points[0];
                var end = stroke.points[stroke.points.length - 1];
                if (stroke.type === "text" || stroke.type === "number") {
                    var textLeft = Math.min(start.x, end.x);
                    var textTop = Math.min(start.y, end.y);
                    var textWidth = Math.abs(end.x - start.x);
                    var textHeight = Math.abs(end.y - start.y);
                    context.save();
                    context.translate(textLeft, textTop);
                    context.scale(textWidth / Math.max(1, stroke.baseWidth),
                                  textHeight / Math.max(1, stroke.baseHeight));
                    context.fillStyle = stroke.color;
                    context.strokeStyle = stroke.color;
                    context.textAlign = stroke.type === "number" ? "center" : "left";
                    context.textBaseline = "middle";
                    var fontFamily = stroke.fontFamily || textEditor.font.family;
                    context.font = "20px \"" + fontFamily.replace(/"/g, "\\\"") + "\"";
                    if (stroke.type === "number") {
                        var radius = stroke.baseWidth / 2 - Math.max(1, stroke.width / 2);
                        context.lineWidth = stroke.width;
                        context.beginPath();
                        context.arc(stroke.baseWidth / 2, stroke.baseHeight / 2,
                                    Math.max(1, radius), 0, Math.PI * 2);
                        context.stroke();
                        context.fillText(stroke.number.toString(), stroke.baseWidth / 2,
                                         stroke.baseHeight / 2);
                    } else {
                        context.fillText(stroke.text, 0, stroke.baseHeight / 2);
                    }
                    context.restore();
                    return;
                }
                if (stroke.type === "effectSelection") {
                    var effectLeft = Math.min(start.x, end.x);
                    var effectTop = Math.min(start.y, end.y);
                    context.save();
                    context.fillStyle = "rgba(47, 128, 237, 0.16)";
                    context.strokeStyle = "#2f80ed";
                    context.lineWidth = 1;
                    context.setLineDash([5, 3]);
                    context.fillRect(effectLeft, effectTop, Math.abs(end.x - start.x), Math.abs(end.y - start.y));
                    context.strokeRect(effectLeft, effectTop, Math.abs(end.x - start.x), Math.abs(end.y - start.y));
                    context.restore();
                    return;
                }
                if (stroke.type === "rect" || stroke.type === "ellipse") {
                    var left = Math.min(start.x, end.x);
                    var top = Math.min(start.y, end.y);
                    var shapeWidth = Math.abs(end.x - start.x);
                    var shapeHeight = Math.abs(end.y - start.y);
                    context.beginPath();
                    context.strokeStyle = stroke.color;
                    context.lineWidth = stroke.width;
                    if (stroke.type === "rect") {
                        context.rect(left, top, shapeWidth, shapeHeight);
                    } else {
                        context.ellipse(left + shapeWidth / 2, top + shapeHeight / 2,
                                        shapeWidth / 2, shapeHeight / 2, 0, 0, Math.PI * 2);
                    }
                    context.stroke();
                    return;
                }
                context.beginPath();
                context.strokeStyle = stroke.color;
                context.lineWidth = stroke.width;
                context.moveTo(start.x, start.y);
                for (var i = 1; i < stroke.points.length; ++i) {
                    context.lineTo(stroke.points[i].x, stroke.points[i].y);
                }
                context.stroke();

                if (stroke.type === "arrow") {
                    var angle = Math.atan2(end.y - start.y, end.x - start.x);
                    var arrowLength = Math.max(10, stroke.width * 4);
                    var spread = Math.PI / 7;
                    context.beginPath();
                    context.moveTo(end.x, end.y);
                    context.lineTo(end.x - arrowLength * Math.cos(angle - spread),
                                   end.y - arrowLength * Math.sin(angle - spread));
                    context.moveTo(end.x, end.y);
                    context.lineTo(end.x - arrowLength * Math.cos(angle + spread),
                                   end.y - arrowLength * Math.sin(angle + spread));
                    context.stroke();
                }
            }

            for (var i = 0; i < editCanvas.strokes.length; ++i) {
                drawStroke(editCanvas.strokes[i]);
            }
            drawStroke({ type: drawingCanvas.interaction === "effect" ? "effectSelection" : editCanvas.currentTool,
                         points: activePoints, color: editCanvas.currentColor.toString(), width: editCanvas.thickness });

            if (editCanvas.selectedIndex >= 0 && editCanvas.selectedIndex < editCanvas.strokes.length) {
                var bounds = editCanvas.boundsFor(editCanvas.strokes[editCanvas.selectedIndex]);
                context.save();
                context.strokeStyle = "#2f80ed";
                context.fillStyle = "#ffffff";
                context.lineWidth = 1;
                context.setLineDash([4, 3]);
                context.strokeRect(bounds.left, bounds.top, Math.max(1, bounds.width), Math.max(1, bounds.height));
                context.setLineDash([]);
                var handles = [
                    [(bounds.left + bounds.right) / 2, bounds.top],
                    [bounds.right, (bounds.top + bounds.bottom) / 2],
                    [(bounds.left + bounds.right) / 2, bounds.bottom],
                    [bounds.left, (bounds.top + bounds.bottom) / 2],
                    [bounds.left, bounds.top],
                    [bounds.right, bounds.top],
                    [bounds.left, bounds.bottom],
                    [bounds.right, bounds.bottom]
                ];
                for (var h = 0; h < handles.length; ++h) {
                    context.beginPath();
                    context.arc(handles[h][0], handles[h][1], 4, 0, Math.PI * 2);
                    context.fill();
                    context.stroke();
                }
                context.restore();
            }

            if (editCanvas.currentTool === "crop" && editCanvas.cropRect.width > 0) {
                var crop = editCanvas.cropRect;
                context.save();
                context.fillStyle = "rgba(0, 0, 0, 0.55)";
                context.fillRect(0, 0, width, crop.y);
                context.fillRect(0, crop.y, crop.x, crop.height);
                context.fillRect(crop.x + crop.width, crop.y,
                                 width - crop.x - crop.width, crop.height);
                context.fillRect(0, crop.y + crop.height, width,
                                 height - crop.y - crop.height);
                context.strokeStyle = "#ffffff";
                context.lineWidth = 1;
                context.strokeRect(crop.x, crop.y, crop.width, crop.height);
                context.strokeStyle = Qt.rgba(palette.light.r,
                                              palette.light.g,
                                              palette.light.b, 0.5);
                context.lineWidth = 1;
                context.beginPath();
                context.moveTo(crop.x, crop.y + crop.height / 3);
                context.lineTo(crop.x + crop.width, crop.y + crop.height / 3);
                context.moveTo(crop.x, crop.y + crop.height / 3 * 2);
                context.lineTo(crop.x + crop.width, crop.y + crop.height / 3 * 2);
                context.moveTo(crop.x + crop.width / 3, crop.y);
                context.lineTo(crop.x + crop.width / 3, crop.y + crop.height);
                context.moveTo(crop.x + crop.width / 3 * 2, crop.y);
                context.lineTo(crop.x + crop.width / 3 * 2, crop.y + crop.height);
                context.stroke();
                var cropHandles = [
                    [crop.x, crop.y], [crop.x + crop.width, crop.y],
                    [crop.x + crop.width, crop.y + crop.height], [crop.x, crop.y + crop.height]
                ];
                context.fillStyle = "#ffffff";
                for (var c = 0; c < cropHandles.length; ++c)
                    context.fillRect(cropHandles[c][0] - 4, cropHandles[c][1] - 4, 8, 8);
                context.restore();
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            enabled: IV.GStatus.editMode && (editCanvas.currentTool === "pen"
                                             || editCanvas.currentTool === "line"
                                             || editCanvas.currentTool === "arrow"
                                             || editCanvas.currentTool === "rect"
                                             || editCanvas.currentTool === "ellipse"
                                             || editCanvas.currentTool === "text"
                                             || editCanvas.currentTool === "blur"
                                             || editCanvas.currentTool === "crop")
            preventStealing: true

            onPressed: mouse => {
                editCanvas.forceActiveFocus();
                drawingCanvas.pressPoint = Qt.point(mouse.x, mouse.y);
                drawingCanvas.interactionChanged = false;
                if (editCanvas.currentTool === "crop") {
                    drawingCanvas.resizeHandle = editCanvas.cropHandleAt(mouse.x, mouse.y);
                    if (drawingCanvas.resizeHandle === "") return;
                    drawingCanvas.interaction = "crop";
                    drawingCanvas.originalPoints = [Qt.point(editCanvas.cropRect.x, editCanvas.cropRect.y),
                                                    Qt.point(editCanvas.cropRect.x + editCanvas.cropRect.width,
                                                             editCanvas.cropRect.y + editCanvas.cropRect.height)];
                    return;
                }
                drawingCanvas.resizeHandle = editCanvas.handleAt(mouse.x, mouse.y);
                if (drawingCanvas.resizeHandle !== "") {
                    drawingCanvas.interaction = "resize";
                    drawingCanvas.originalPoints = editCanvas.strokes[editCanvas.selectedIndex].points.slice();
                    return;
                }
                var hitIndex = editCanvas.strokeAt(mouse.x, mouse.y);
                if (hitIndex >= 0) {
                    editCanvas.selectedIndex = hitIndex;
                    drawingCanvas.interaction = "move";
                    drawingCanvas.originalPoints = editCanvas.strokes[hitIndex].points.slice();
                    drawingCanvas.requestPaint();
                    return;
                }
                editCanvas.selectedIndex = -1;
                if (editCanvas.currentTool === "blur") {
                    drawingCanvas.interaction = "effect";
                    drawingCanvas.activePoints = [Qt.point(mouse.x, mouse.y), Qt.point(mouse.x, mouse.y)];
                    drawingCanvas.requestPaint();
                    return;
                }
                if (editCanvas.currentTool === "text") {
                    drawingCanvas.interaction = "";
                    if (editCanvas.textMode === "number") {
                        editCanvas.addNumber(mouse.x, mouse.y);
                    } else {
                        textEditor.x = mouse.x;
                        textEditor.y = mouse.y;
                        textEditor.visible = true;
                        textEditor.forceActiveFocus();
                    }
                    drawingCanvas.requestPaint();
                    return;
                }
                drawingCanvas.interaction = "draw";
                drawingCanvas.activePoints = [Qt.point(mouse.x, mouse.y)];
                drawingCanvas.requestPaint();
            }

            onDoubleClicked: mouse => {
                if (editCanvas.currentTool === "crop"
                        && mouse.x >= editCanvas.cropRect.x
                        && mouse.x <= editCanvas.cropRect.x + editCanvas.cropRect.width
                        && mouse.y >= editCanvas.cropRect.y
                        && mouse.y <= editCanvas.cropRect.y + editCanvas.cropRect.height)
                    editCanvas.confirmCrop();
            }

            onPositionChanged: mouse => {
                if (!pressed) return;
                if (drawingCanvas.interaction === "move") {
                    drawingCanvas.interactionChanged = true;
                    var moved = [];
                    var deltaX = mouse.x - drawingCanvas.pressPoint.x;
                    var deltaY = mouse.y - drawingCanvas.pressPoint.y;
                    for (var i = 0; i < drawingCanvas.originalPoints.length; ++i)
                        moved.push(Qt.point(drawingCanvas.originalPoints[i].x + deltaX,
                                            drawingCanvas.originalPoints[i].y + deltaY));
                    var movedStroke = Object.assign({}, editCanvas.strokes[editCanvas.selectedIndex], { points: moved });
                    editCanvas.replaceStroke(editCanvas.selectedIndex, movedStroke);
                    return;
                }
                if (drawingCanvas.interaction === "resize") {
                    drawingCanvas.interactionChanged = true;
                    var selected = editCanvas.strokes[editCanvas.selectedIndex];
                    var originalBounds = editCanvas.boundsFor({ points: drawingCanvas.originalPoints });
                    var left = originalBounds.left;
                    var right = originalBounds.right;
                    var top = originalBounds.top;
                    var bottom = originalBounds.bottom;
                    var handle = drawingCanvas.resizeHandle;
                    var resizedPoints = [];
                    if (selected.type === "rect" || selected.type === "ellipse") {
                        if (handle.indexOf("Left") >= 0) left = mouse.x;
                        if (handle.indexOf("Right") >= 0) right = mouse.x;
                        if (handle.indexOf("top") === 0) top = mouse.y;
                        if (handle.indexOf("bottom") === 0) bottom = mouse.y;
                        if (handle === "left") left = mouse.x;
                        else if (handle === "right") right = mouse.x;
                        else if (handle === "top") top = mouse.y;
                        else if (handle === "bottom") bottom = mouse.y;
                        resizedPoints = [Qt.point(left, top), Qt.point(right, bottom)];
                    } else {
                        var anchorX = (left + right) / 2;
                        var anchorY = (top + bottom) / 2;
                        var scaleX = 1;
                        var scaleY = 1;
                        if (handle === "left" || handle === "topLeft" || handle === "bottomLeft") {
                            if (originalBounds.width > 0) {
                                anchorX = right;
                                scaleX = (right - mouse.x) / originalBounds.width;
                            }
                        } else if (handle === "right" || handle === "topRight" || handle === "bottomRight") {
                            if (originalBounds.width > 0) {
                                anchorX = left;
                                scaleX = (mouse.x - left) / originalBounds.width;
                            }
                        }
                        if (handle === "top" || handle === "topLeft" || handle === "topRight") {
                            if (originalBounds.height > 0) {
                                anchorY = bottom;
                                scaleY = (bottom - mouse.y) / originalBounds.height;
                            }
                        } else if (handle === "bottom" || handle === "bottomLeft" || handle === "bottomRight") {
                            if (originalBounds.height > 0) {
                                anchorY = top;
                                scaleY = (mouse.y - top) / originalBounds.height;
                            }
                        }
                        scaleX = Math.max(0.05, scaleX);
                        scaleY = Math.max(0.05, scaleY);
                        for (var p = 0; p < drawingCanvas.originalPoints.length; ++p) {
                            var original = drawingCanvas.originalPoints[p];
                            resizedPoints.push(Qt.point(anchorX + (original.x - anchorX) * scaleX,
                                                        anchorY + (original.y - anchorY) * scaleY));
                        }
                    }
                    var resized = Object.assign({}, selected, { points: resizedPoints });
                    editCanvas.replaceStroke(editCanvas.selectedIndex, resized);
                    return;
                }
                if (drawingCanvas.interaction === "effect") {
                    drawingCanvas.activePoints = [drawingCanvas.pressPoint, Qt.point(mouse.x, mouse.y)];
                    drawingCanvas.requestPaint();
                    return;
                }
                if (drawingCanvas.interaction === "crop") {
                    var originalLeft = drawingCanvas.originalPoints[0].x;
                    var originalTop = drawingCanvas.originalPoints[0].y;
                    var originalRight = drawingCanvas.originalPoints[1].x;
                    var originalBottom = drawingCanvas.originalPoints[1].y;
                    var cropDeltaX = mouse.x - drawingCanvas.pressPoint.x;
                    var cropDeltaY = mouse.y - drawingCanvas.pressPoint.y;
                    var minSize = 16;
                    if (drawingCanvas.resizeHandle === "move") {
                        var moveX = Math.max(-originalLeft,
                                             Math.min(editCanvas.width - originalRight, cropDeltaX));
                        var moveY = Math.max(-originalTop,
                                             Math.min(editCanvas.height - originalBottom, cropDeltaY));
                        originalLeft += moveX;
                        originalRight += moveX;
                        originalTop += moveY;
                        originalBottom += moveY;
                    } else {
                        if (drawingCanvas.resizeHandle.indexOf("Left") >= 0)
                            originalLeft = Math.max(0, Math.min(originalRight - minSize, mouse.x));
                        if (drawingCanvas.resizeHandle.indexOf("Right") >= 0)
                            originalRight = Math.min(editCanvas.width, Math.max(originalLeft + minSize, mouse.x));
                        if (drawingCanvas.resizeHandle.indexOf("top") === 0)
                            originalTop = Math.max(0, Math.min(originalBottom - minSize, mouse.y));
                        if (drawingCanvas.resizeHandle.indexOf("bottom") === 0)
                            originalBottom = Math.min(editCanvas.height, Math.max(originalTop + minSize, mouse.y));
                    }
                    editCanvas.cropRect = Qt.rect(originalLeft, originalTop,
                                                  originalRight - originalLeft, originalBottom - originalTop);
                    drawingCanvas.requestPaint();
                    return;
                }
                var points = drawingCanvas.activePoints.slice();
                if (editCanvas.currentTool === "line" || editCanvas.currentTool === "arrow") {
                    drawingCanvas.activePoints = [points[0], Qt.point(mouse.x, mouse.y)];
                    drawingCanvas.requestPaint();
                    return;
                }
                if (editCanvas.currentTool === "rect" || editCanvas.currentTool === "ellipse") {
                    var start = points[0];
                    var deltaX = mouse.x - start.x;
                    var deltaY = mouse.y - start.y;
                    var side = Math.min(Math.abs(deltaX), Math.abs(deltaY));
                    var endX = start.x + (deltaX < 0 ? -side : side);
                    var endY = start.y + (deltaY < 0 ? -side : side);
                    drawingCanvas.activePoints = [start, Qt.point(endX, endY)];
                    drawingCanvas.requestPaint();
                    return;
                }
                var previous = points[points.length - 1];
                if (Math.abs(mouse.x - previous.x) + Math.abs(mouse.y - previous.y) < 1) return;
                points.push(Qt.point(mouse.x, mouse.y));
                drawingCanvas.activePoints = points;
                drawingCanvas.requestPaint();
            }

            onReleased: {
                if (drawingCanvas.interaction === "crop") {
                    drawingCanvas.interaction = "";
                    return;
                }
                if (drawingCanvas.interaction === "effect") {
                    var effectStart = drawingCanvas.activePoints[0];
                    var effectEnd = drawingCanvas.activePoints[1];
                    var left = Math.min(effectStart.x, effectEnd.x);
                    var top = Math.min(effectStart.y, effectEnd.y);
                    var effectWidth = Math.abs(effectEnd.x - effectStart.x);
                    var effectHeight = Math.abs(effectEnd.y - effectStart.y);
                    if (effectWidth >= 2 && effectHeight >= 2 && editCanvas.width > 0 && editCanvas.height > 0) {
                        editCanvas.effectRequested(editCanvas.blurMode,
                                                   Qt.rect(left / editCanvas.width, top / editCanvas.height,
                                                           effectWidth / editCanvas.width, effectHeight / editCanvas.height),
                                                   editCanvas.effectStrength);
                    }
                    drawingCanvas.interaction = "";
                    drawingCanvas.activePoints = [];
                    drawingCanvas.requestPaint();
                    return;
                }
                if (drawingCanvas.interaction === "move" || drawingCanvas.interaction === "resize") {
                    drawingCanvas.interaction = "";
                    if (drawingCanvas.interactionChanged) editCanvas.strokeCommitted();
                    return;
                }
                if (drawingCanvas.activePoints.length >= 2) {
                    var first = drawingCanvas.activePoints[0];
                    var last = drawingCanvas.activePoints[drawingCanvas.activePoints.length - 1];
                    if (Math.abs(last.x - first.x) + Math.abs(last.y - first.y) < 1) {
                        drawingCanvas.activePoints = [];
                        drawingCanvas.requestPaint();
                        return;
                    }
                    var updated = editCanvas.strokes.slice();
                    updated.push({
                        type: editCanvas.currentTool,
                        points: drawingCanvas.activePoints.slice(),
                        color: editCanvas.currentColor.toString(),
                        width: editCanvas.thickness
                    });
                    editCanvas.strokes = updated;
                    editCanvas.selectedIndex = updated.length - 1;
                    editCanvas.strokeCommitted();
                }
                drawingCanvas.interaction = "";
                drawingCanvas.activePoints = [];
                drawingCanvas.requestPaint();
            }

            onCanceled: {
                if (drawingCanvas.interaction === "crop") {
                    editCanvas.cropRect = Qt.rect(drawingCanvas.originalPoints[0].x,
                                                  drawingCanvas.originalPoints[0].y,
                                                  drawingCanvas.originalPoints[1].x - drawingCanvas.originalPoints[0].x,
                                                  drawingCanvas.originalPoints[1].y - drawingCanvas.originalPoints[0].y);
                }
                if ((drawingCanvas.interaction === "move" || drawingCanvas.interaction === "resize")
                        && editCanvas.selectedIndex >= 0) {
                    var restored = Object.assign({}, editCanvas.strokes[editCanvas.selectedIndex],
                                                 { points: drawingCanvas.originalPoints.slice() });
                    editCanvas.replaceStroke(editCanvas.selectedIndex, restored);
                }
                drawingCanvas.interaction = "";
                drawingCanvas.activePoints = [];
                drawingCanvas.requestPaint();
            }
        }
    }

    TextInput {
        id: textEditor

        color: editCanvas.currentColor
        font.pixelSize: 20
        selectByMouse: true
        visible: false
        z: 2

        Keys.onEscapePressed: {
            text = "";
            visible = false;
            editCanvas.forceActiveFocus();
        }
        Keys.onReturnPressed: editCanvas.commitTextInput()
        onActiveFocusChanged: {
            if (!activeFocus && visible) editCanvas.commitTextInput();
        }
    }

    focus: visible
    Keys.onDeletePressed: removeSelected()
    Keys.onReturnPressed: confirmCrop()
}
