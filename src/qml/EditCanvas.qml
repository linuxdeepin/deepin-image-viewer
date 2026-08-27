// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0 as DTK
import org.deepin.image.viewer 1.0 as IV

Item {
    id: editCanvas

    Accessible.name: "EditCanvas" + strokes.length
    Accessible.role: Accessible.Pane

    property color currentColor: "#e53935"
    property string currentTool: ""
    property int effectStrength: 15
    property color selectionActiveColor: "#2f80ed"
    property int selectedIndex: -1
    property var strokes: []
    property string blurMode: "gaussian"
    property rect cropRect: Qt.rect(0, 0, 0, 0)
    property real previousCanvasHeight: 0
    property real previousCanvasWidth: 0
    readonly property real stableCanvasHeight: height > 0 ? height : previousCanvasHeight
    readonly property real stableCanvasWidth: width > 0 ? width : previousCanvasWidth
    property string textMode: "plain"
    property int thickness: 5
    property var vectorHistory: []
    property int vectorHistoryIndex: -1
    readonly property real numberTextContrastThreshold: 170
    readonly property real numberTextHorizontalOffsetRatio: -0.025
    readonly property string numberFontFamily: Qt.fontFamilies().indexOf("Source Han Sans SC") >= 0
                                               ? "Source Han Sans SC" : textEditor.font.family

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
                points.push(normalize ? Qt.point(pointX / Math.max(1, stableCanvasWidth),
                                                 pointY / Math.max(1, stableCanvasHeight))
                                      : Qt.point(pointX * stableCanvasWidth,
                                                 pointY * stableCanvasHeight));
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

    function rotatePoint(point, centerX, centerY, degrees) {
        var radians = degrees * Math.PI / 180;
        var cosine = Math.cos(radians);
        var sine = Math.sin(radians);
        var deltaX = point.x - centerX;
        var deltaY = point.y - centerY;
        return Qt.point(centerX + deltaX * cosine - deltaY * sine,
                        centerY + deltaX * sine + deltaY * cosine);
    }

    function handlesForStroke(stroke) {
        if (!stroke || stroke.points.length < 2) return [];
        if (stroke.type === "line" || stroke.type === "arrow") {
            return [
                { name: "start", x: stroke.points[0].x, y: stroke.points[0].y },
                { name: "end", x: stroke.points[stroke.points.length - 1].x,
                  y: stroke.points[stroke.points.length - 1].y }
            ];
        }

        var bounds = boundsFor(stroke);
        var centerX = (bounds.left + bounds.right) / 2;
        var centerY = (bounds.top + bounds.bottom) / 2;
        var handles = stroke.type === "text" || stroke.type === "number" ? [
            { name: "topLeft", x: bounds.left, y: bounds.top },
            { name: "topRight", x: bounds.right, y: bounds.top },
            { name: "bottomLeft", x: bounds.left, y: bounds.bottom },
            { name: "bottomRight", x: bounds.right, y: bounds.bottom }
        ] : [
            { name: "top", x: centerX, y: bounds.top },
            { name: "right", x: bounds.right, y: centerY },
            { name: "bottom", x: centerX, y: bounds.bottom },
            { name: "left", x: bounds.left, y: centerY },
            { name: "topLeft", x: bounds.left, y: bounds.top },
            { name: "topRight", x: bounds.right, y: bounds.top },
            { name: "bottomLeft", x: bounds.left, y: bounds.bottom },
            { name: "bottomRight", x: bounds.right, y: bounds.bottom }
        ];

        var rotation = Number(stroke.rotation || 0);
        if ((stroke.type === "rect" || stroke.type === "ellipse") && rotation !== 0) {
            for (var i = 0; i < handles.length; ++i) {
                var rotated = rotatePoint(Qt.point(handles[i].x, handles[i].y),
                                          centerX, centerY, rotation);
                handles[i].x = rotated.x;
                handles[i].y = rotated.y;
            }
        }
        if (stroke.type === "pen" || stroke.type === "rect" || stroke.type === "ellipse") {
            var rotationHandle = rotatePoint(Qt.point(centerX, bounds.top - 24),
                                             centerX, centerY, rotation);
            handles.push({ name: "rotate", x: rotationHandle.x, y: rotationHandle.y });
        }
        return handles;
    }

    function handleAt(x, y) {
        if (selectedIndex < 0 || selectedIndex >= strokes.length) return "";
        var handles = handlesForStroke(strokes[selectedIndex]);
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
                var rectPoint = rotatePoint(Qt.point(x, y),
                                            (bounds.left + bounds.right) / 2,
                                            (bounds.top + bounds.bottom) / 2,
                                            -Number(stroke.rotation || 0));
                var insideX = rectPoint.x >= bounds.left - tolerance
                        && rectPoint.x <= bounds.right + tolerance;
                var insideY = rectPoint.y >= bounds.top - tolerance
                        && rectPoint.y <= bounds.bottom + tolerance;
                var nearVertical = Math.abs(rectPoint.x - bounds.left) <= tolerance
                        || Math.abs(rectPoint.x - bounds.right) <= tolerance;
                var nearHorizontal = Math.abs(rectPoint.y - bounds.top) <= tolerance
                        || Math.abs(rectPoint.y - bounds.bottom) <= tolerance;
                if ((insideY && nearVertical) || (insideX && nearHorizontal)) return i;
                continue;
            }
            if (stroke.type === "ellipse") {
                var ellipsePoint = rotatePoint(Qt.point(x, y),
                                               (bounds.left + bounds.right) / 2,
                                               (bounds.top + bounds.bottom) / 2,
                                               -Number(stroke.rotation || 0));
                var radiusX = Math.max(bounds.width / 2, 0.5);
                var radiusY = Math.max(bounds.height / 2, 0.5);
                var normalizedX = (ellipsePoint.x - (bounds.left + bounds.right) / 2) / radiusX;
                var normalizedY = (ellipsePoint.y - (bounds.top + bounds.bottom) / 2) / radiusY;
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

    function rotateClockwise() {
        // Rotate normalized coordinates here; the canvas resize handlers scale them
        // into the image's swapped dimensions after the rotated image is reloaded.
        // Rectangles and ellipses retain their angle because swapping their bounds
        // already represents the additional 90-degree rotation.
        var canvasWidth = stableCanvasWidth;
        var canvasHeight = stableCanvasHeight;
        if (canvasWidth <= 0 || canvasHeight <= 0) return;
        var updated = [];
        for (var i = 0; i < strokes.length; ++i) {
            var points = [];
            for (var p = 0; p < strokes[i].points.length; ++p) {
                var normalizedX = strokes[i].points[p].x / canvasWidth;
                var normalizedY = strokes[i].points[p].y / canvasHeight;
                points.push(Qt.point((1 - normalizedY) * canvasWidth,
                                     normalizedX * canvasHeight));
            }
            updated.push(Object.assign({}, strokes[i], { points: points }));
        }
        strokes = updated;
        selectedIndex = -1;
        strokeCommitted();
        drawingCanvas.requestPaint();
    }

    function updateSelectedStyle(color, width) {
        if (selectedIndex < 0 || selectedIndex >= strokes.length) return;
        var changes = { color: color.toString() };
        if (strokes[selectedIndex].type !== "number") changes.width = width;
        var stroke = Object.assign({}, strokes[selectedIndex], changes);
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

    function numberTextColor(colorValue) {
        // Stroke colors are normalized through the QML color type before storage.
        var hex = colorValue.toString().toLowerCase();
        if (hex.length !== 7 || hex.charAt(0) !== "#") return "#ffffff";
        var red = parseInt(hex.substr(1, 2), 16);
        var green = parseInt(hex.substr(3, 2), 16);
        var blue = parseInt(hex.substr(5, 2), 16);
        return red * 0.299 + green * 0.587 + blue * 0.114 > numberTextContrastThreshold
                ? "#000000" : "#ffffff";
    }

    function addNumber(x, y) {
        var diameter = 20;
        var updated = strokes.slice();
        updated.push({
            type: "number",
            number: nextNumber(),
            points: [Qt.point(x - diameter / 2, y - diameter / 2),
                     Qt.point(x + diameter / 2, y + diameter / 2)],
            color: currentColor.toString(),
            fontFamily: numberFontFamily,
            width: 0,
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
                points.push(Qt.point(strokes[i].points[p].x / Math.max(1, stableCanvasWidth),
                                     strokes[i].points[p].y / Math.max(1, stableCanvasHeight)));
            }
            annotations.push(Object.assign({}, strokes[i], {
                points: points,
                width: strokes[i].width / Math.max(1, stableCanvasWidth)
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

    function cursorForHandle(handle) {
        if (handle === "rotate") return Qt.BlankCursor;
        if (handle === "start" || handle === "end") {
            if (selectedIndex < 0 || selectedIndex >= strokes.length) return Qt.ArrowCursor;
            var selected = strokes[selectedIndex];
            var first = selected.points[0];
            var last = selected.points[selected.points.length - 1];
            var angle = Math.abs(Math.atan2(last.y - first.y, last.x - first.x) * 180 / Math.PI);
            if (angle <= 22.5 || angle >= 157.5) return Qt.SizeHorCursor;
            if (angle >= 67.5 && angle <= 112.5) return Qt.SizeVerCursor;
            return angle < 67.5 ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor;
        }
        if (handle === "left" || handle === "right") return Qt.SizeHorCursor;
        if (handle === "top" || handle === "bottom") return Qt.SizeVerCursor;
        if (handle === "topLeft" || handle === "bottomRight") return Qt.SizeFDiagCursor;
        if (handle === "topRight" || handle === "bottomLeft") return Qt.SizeBDiagCursor;
        if (handle === "move") return Qt.SizeAllCursor;
        return Qt.ArrowCursor;
    }

    function resizedBounds(bounds, handle, mouseX, mouseY, proportional) {
        var left = bounds.left;
        var right = bounds.right;
        var top = bounds.top;
        var bottom = bounds.bottom;
        var changesLeft = handle === "left" || handle === "topLeft" || handle === "bottomLeft";
        var changesRight = handle === "right" || handle === "topRight" || handle === "bottomRight";
        var changesTop = handle === "top" || handle === "topLeft" || handle === "topRight";
        var changesBottom = handle === "bottom" || handle === "bottomLeft" || handle === "bottomRight";

        if (!proportional) {
            if (changesLeft) left = Math.min(right - 1, mouseX);
            if (changesRight) right = Math.max(left + 1, mouseX);
            if (changesTop) top = Math.min(bottom - 1, mouseY);
            if (changesBottom) bottom = Math.max(top + 1, mouseY);
            return { left: left, right: right, top: top, bottom: bottom,
                     width: right - left, height: bottom - top };
        }

        var originalWidth = Math.max(1, bounds.width);
        var originalHeight = Math.max(1, bounds.height);
        var centerX = (bounds.left + bounds.right) / 2;
        var centerY = (bounds.top + bounds.bottom) / 2;
        var scale = 1;
        if (changesLeft && !changesTop && !changesBottom) {
            scale = Math.max(0.05, (bounds.right - mouseX) / originalWidth);
            left = bounds.right - originalWidth * scale;
            top = centerY - originalHeight * scale / 2;
            bottom = centerY + originalHeight * scale / 2;
        } else if (changesRight && !changesTop && !changesBottom) {
            scale = Math.max(0.05, (mouseX - bounds.left) / originalWidth);
            right = bounds.left + originalWidth * scale;
            top = centerY - originalHeight * scale / 2;
            bottom = centerY + originalHeight * scale / 2;
        } else if (changesTop && !changesLeft && !changesRight) {
            scale = Math.max(0.05, (bounds.bottom - mouseY) / originalHeight);
            top = bounds.bottom - originalHeight * scale;
            left = centerX - originalWidth * scale / 2;
            right = centerX + originalWidth * scale / 2;
        } else if (changesBottom && !changesLeft && !changesRight) {
            scale = Math.max(0.05, (mouseY - bounds.top) / originalHeight);
            bottom = bounds.top + originalHeight * scale;
            left = centerX - originalWidth * scale / 2;
            right = centerX + originalWidth * scale / 2;
        } else {
            var anchorX = changesLeft ? bounds.right : bounds.left;
            var anchorY = changesTop ? bounds.bottom : bounds.top;
            scale = Math.max(0.05,
                             Math.max(Math.abs(mouseX - anchorX) / originalWidth,
                                      Math.abs(mouseY - anchorY) / originalHeight));
            var newWidth = originalWidth * scale;
            var newHeight = originalHeight * scale;
            left = changesLeft ? anchorX - newWidth : anchorX;
            right = changesLeft ? anchorX : anchorX + newWidth;
            top = changesTop ? anchorY - newHeight : anchorY;
            bottom = changesTop ? anchorY : anchorY + newHeight;
        }
        return { left: left, right: right, top: top, bottom: bottom,
                 width: right - left, height: bottom - top };
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
        if (width <= 0) return;
        if (previousCanvasWidth > 0) scaleStrokes(width / previousCanvasWidth, 1);
        previousCanvasWidth = width;
        if (currentTool === "crop") resetCrop();
        drawingCanvas.requestPaint();
    }
    onHeightChanged: {
        if (height <= 0) return;
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
        property string hoveredHandle: ""
        property string resizeHandle: ""
        property point pressPoint: Qt.point(0, 0)
        property var originalPoints: []
        property real originalRotation: 0
        property point rotationCenter: Qt.point(0, 0)
        property real rotationStartAngle: 0

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
                    context.textAlign = stroke.type === "number" ? "center" : "left";
                    context.textBaseline = "middle";
                    var fontFamily = stroke.fontFamily || textEditor.font.family;
                    if (stroke.type === "number") {
                        context.fillStyle = stroke.color;
                        context.beginPath();
                        context.arc(stroke.baseWidth / 2, stroke.baseHeight / 2,
                                    stroke.baseWidth / 2, 0, Math.PI * 2);
                        context.fill();
                        context.fillStyle = editCanvas.numberTextColor(stroke.color);
                        context.font = "500 12px \"" + fontFamily.replace(/"/g, "\\\"") + "\"";
                        context.fillText(stroke.number.toString(),
                                         stroke.baseWidth * (0.5 + numberTextHorizontalOffsetRatio),
                                         stroke.baseHeight / 2);
                    } else {
                        context.fillStyle = stroke.color;
                        context.font = "20px \"" + fontFamily.replace(/"/g, "\\\"") + "\"";
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
                    var centerX = left + shapeWidth / 2;
                    var centerY = top + shapeHeight / 2;
                    context.save();
                    context.translate(centerX, centerY);
                    context.rotate(Number(stroke.rotation || 0) * Math.PI / 180);
                    context.beginPath();
                    context.strokeStyle = stroke.color;
                    context.lineWidth = stroke.width;
                    if (stroke.type === "rect") {
                        context.rect(-shapeWidth / 2, -shapeHeight / 2, shapeWidth, shapeHeight);
                    } else {
                        context.ellipse(-shapeWidth / 2, -shapeHeight / 2, shapeWidth, shapeHeight);
                    }
                    context.stroke();
                    context.restore();
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
                var selectedStroke = editCanvas.strokes[editCanvas.selectedIndex];
                var bounds = editCanvas.boundsFor(selectedStroke);
                var selectionHandles = editCanvas.handlesForStroke(selectedStroke);
                var selectionColor = drawingCanvas.hoveredHandle !== ""
                        ? editCanvas.selectionActiveColor : "#ffffff";
                context.save();
                context.strokeStyle = selectionColor;
                context.fillStyle = "#ffffff";
                context.lineWidth = 1;
                if (selectedStroke.type !== "line" && selectedStroke.type !== "arrow") {
                    if (selectedStroke.type === "rect" || selectedStroke.type === "ellipse") {
                        var selectionCenterX = (bounds.left + bounds.right) / 2;
                        var selectionCenterY = (bounds.top + bounds.bottom) / 2;
                        context.translate(selectionCenterX, selectionCenterY);
                        context.rotate(Number(selectedStroke.rotation || 0) * Math.PI / 180);
                        context.strokeRect(-Math.max(1, bounds.width) / 2,
                                           -Math.max(1, bounds.height) / 2,
                                           Math.max(1, bounds.width), Math.max(1, bounds.height));
                        context.restore();
                        context.save();
                        context.strokeStyle = selectionColor;
                        context.fillStyle = "#ffffff";
                        context.lineWidth = 1;
                    } else {
                        context.strokeRect(bounds.left, bounds.top,
                                           Math.max(1, bounds.width), Math.max(1, bounds.height));
                    }
                }
                for (var handleIndex = 0; handleIndex < selectionHandles.length; ++handleIndex) {
                    var selectionHandle = selectionHandles[handleIndex];
                    if (selectionHandle.name === "rotate") continue;
                    context.fillStyle = selectionHandle.name === drawingCanvas.hoveredHandle
                            ? editCanvas.selectionActiveColor : "#ffffff";
                    context.fillRect(selectionHandle.x - 4, selectionHandle.y - 4, 8, 8);
                    context.strokeRect(selectionHandle.x - 4, selectionHandle.y - 4, 8, 8);
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
            id: canvasMouseArea

            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            cursorShape: {
                if (drawingCanvas.interaction === "rotate") return Qt.BlankCursor;
                if (editCanvas.currentTool === "crop")
                    return editCanvas.cursorForHandle(editCanvas.cropHandleAt(mouseX, mouseY));
                var handle = editCanvas.handleAt(mouseX, mouseY);
                if (handle !== "") return editCanvas.cursorForHandle(handle);
                return editCanvas.strokeAt(mouseX, mouseY) >= 0 ? Qt.SizeAllCursor : Qt.ArrowCursor;
            }
            enabled: IV.GStatus.editMode && (editCanvas.currentTool === "pen"
                                             || editCanvas.currentTool === "line"
                                             || editCanvas.currentTool === "arrow"
                                             || editCanvas.currentTool === "rect"
                                             || editCanvas.currentTool === "ellipse"
                                             || editCanvas.currentTool === "text"
                                             || editCanvas.currentTool === "blur"
                                             || editCanvas.currentTool === "crop")
            hoverEnabled: true
            preventStealing: true

            onPressed: mouse => {
                editCanvas.forceActiveFocus();
                drawingCanvas.pressPoint = Qt.point(mouse.x, mouse.y);
                drawingCanvas.interactionChanged = false;
                drawingCanvas.hoveredHandle = "";
                if (editCanvas.currentTool === "crop") {
                    drawingCanvas.resizeHandle = editCanvas.cropHandleAt(mouse.x, mouse.y);
                    if (drawingCanvas.resizeHandle === "") return;
                    drawingCanvas.interaction = "crop";
                    drawingCanvas.originalPoints = [Qt.point(editCanvas.cropRect.x, editCanvas.cropRect.y),
                                                    Qt.point(editCanvas.cropRect.x + editCanvas.cropRect.width,
                                                             editCanvas.cropRect.y + editCanvas.cropRect.height)];
                    return;
                }
                if (editCanvas.currentTool === "blur") {
                    editCanvas.selectedIndex = -1;
                    drawingCanvas.interaction = "effect";
                    drawingCanvas.activePoints = [Qt.point(mouse.x, mouse.y), Qt.point(mouse.x, mouse.y)];
                    drawingCanvas.requestPaint();
                    return;
                }
                drawingCanvas.resizeHandle = editCanvas.handleAt(mouse.x, mouse.y);
                if (drawingCanvas.resizeHandle !== "") {
                    var selectedStroke = editCanvas.strokes[editCanvas.selectedIndex];
                    drawingCanvas.originalPoints = selectedStroke.points.slice();
                    drawingCanvas.originalRotation = Number(selectedStroke.rotation || 0);
                    if (drawingCanvas.resizeHandle === "rotate") {
                        var rotationBounds = editCanvas.boundsFor(selectedStroke);
                        drawingCanvas.rotationCenter = Qt.point(
                                    (rotationBounds.left + rotationBounds.right) / 2,
                                    (rotationBounds.top + rotationBounds.bottom) / 2);
                        drawingCanvas.rotationStartAngle = Math.atan2(
                                    mouse.y - drawingCanvas.rotationCenter.y,
                                    mouse.x - drawingCanvas.rotationCenter.x);
                        drawingCanvas.interaction = "rotate";
                    } else {
                        drawingCanvas.interaction = "resize";
                    }
                    return;
                }
                var hitIndex = editCanvas.strokeAt(mouse.x, mouse.y);
                if (hitIndex >= 0) {
                    editCanvas.selectedIndex = hitIndex;
                    drawingCanvas.interaction = "move";
                    drawingCanvas.originalPoints = editCanvas.strokes[hitIndex].points.slice();
                    drawingCanvas.originalRotation = Number(editCanvas.strokes[hitIndex].rotation || 0);
                    drawingCanvas.requestPaint();
                    return;
                }
                editCanvas.selectedIndex = -1;
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
                if (!pressed) {
                    var hovered = editCanvas.handleAt(mouse.x, mouse.y);
                    if (hovered !== drawingCanvas.hoveredHandle) {
                        drawingCanvas.hoveredHandle = hovered;
                        drawingCanvas.requestPaint();
                    }
                    return;
                }
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
                if (drawingCanvas.interaction === "rotate") {
                    drawingCanvas.interactionChanged = true;
                    var rotatingStroke = editCanvas.strokes[editCanvas.selectedIndex];
                    var currentAngle = Math.atan2(mouse.y - drawingCanvas.rotationCenter.y,
                                                  mouse.x - drawingCanvas.rotationCenter.x);
                    var angleDelta = (currentAngle - drawingCanvas.rotationStartAngle) * 180 / Math.PI;
                    if (rotatingStroke.type === "pen") {
                        var rotatedPoints = [];
                        for (var rotationIndex = 0;
                             rotationIndex < drawingCanvas.originalPoints.length; ++rotationIndex) {
                            rotatedPoints.push(editCanvas.rotatePoint(
                                                   drawingCanvas.originalPoints[rotationIndex],
                                                   drawingCanvas.rotationCenter.x,
                                                   drawingCanvas.rotationCenter.y,
                                                   angleDelta));
                        }
                        editCanvas.replaceStroke(editCanvas.selectedIndex,
                                                 Object.assign({}, rotatingStroke,
                                                               { points: rotatedPoints }));
                    } else {
                        editCanvas.replaceStroke(editCanvas.selectedIndex,
                                                 Object.assign({}, rotatingStroke,
                                                               { rotation: drawingCanvas.originalRotation
                                                                           + angleDelta }));
                    }
                    return;
                }
                if (drawingCanvas.interaction === "resize") {
                    drawingCanvas.interactionChanged = true;
                    var selected = editCanvas.strokes[editCanvas.selectedIndex];
                    var originalBounds = editCanvas.boundsFor({ points: drawingCanvas.originalPoints });
                    var handle = drawingCanvas.resizeHandle;
                    var resizedPoints = [];
                    var shiftPressed = (mouse.modifiers & Qt.ShiftModifier) !== 0;
                    if (selected.type === "line" || selected.type === "arrow") {
                        resizedPoints = drawingCanvas.originalPoints.slice();
                        var endpoint = Qt.point(mouse.x, mouse.y);
                        var fixedPoint = handle === "start"
                                ? resizedPoints[resizedPoints.length - 1] : resizedPoints[0];
                        if (shiftPressed) {
                            if (Math.abs(endpoint.x - fixedPoint.x) >= Math.abs(endpoint.y - fixedPoint.y))
                                endpoint.y = fixedPoint.y;
                            else
                                endpoint.x = fixedPoint.x;
                        }
                        if (handle === "start") resizedPoints[0] = endpoint;
                        else resizedPoints[resizedPoints.length - 1] = endpoint;
                    } else {
                        var proportional = selected.type === "text" || selected.type === "number"
                                || ((selected.type === "pen" || selected.type === "rect"
                                     || selected.type === "ellipse") && shiftPressed);
                        var mousePoint = Qt.point(mouse.x, mouse.y);
                        var rotation = Number(selected.rotation || 0);
                        var originalCenterX = (originalBounds.left + originalBounds.right) / 2;
                        var originalCenterY = (originalBounds.top + originalBounds.bottom) / 2;
                        if ((selected.type === "rect" || selected.type === "ellipse") && rotation !== 0)
                            mousePoint = editCanvas.rotatePoint(mousePoint, originalCenterX,
                                                               originalCenterY, -rotation);
                        var targetBounds = editCanvas.resizedBounds(originalBounds, handle,
                                                                   mousePoint.x, mousePoint.y,
                                                                   proportional);
                        if (selected.type === "rect" || selected.type === "ellipse") {
                            var localCenter = Qt.point((targetBounds.left + targetBounds.right) / 2,
                                                       (targetBounds.top + targetBounds.bottom) / 2);
                            var worldCenter = editCanvas.rotatePoint(localCenter, originalCenterX,
                                                                     originalCenterY, rotation);
                            resizedPoints = [
                                Qt.point(worldCenter.x - targetBounds.width / 2,
                                         worldCenter.y - targetBounds.height / 2),
                                Qt.point(worldCenter.x + targetBounds.width / 2,
                                         worldCenter.y + targetBounds.height / 2)
                            ];
                        } else {
                            for (var p = 0; p < drawingCanvas.originalPoints.length; ++p) {
                                var original = drawingCanvas.originalPoints[p];
                                var normalizedX = originalBounds.width > 0
                                        ? (original.x - originalBounds.left) / originalBounds.width : 0.5;
                                var normalizedY = originalBounds.height > 0
                                        ? (original.y - originalBounds.top) / originalBounds.height : 0.5;
                                resizedPoints.push(Qt.point(targetBounds.left
                                                            + normalizedX * targetBounds.width,
                                                            targetBounds.top
                                                            + normalizedY * targetBounds.height));
                            }
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
                    var lineEnd = Qt.point(mouse.x, mouse.y);
                    if ((mouse.modifiers & Qt.ShiftModifier) !== 0) {
                        if (Math.abs(lineEnd.x - points[0].x) >= Math.abs(lineEnd.y - points[0].y))
                            lineEnd.y = points[0].y;
                        else
                            lineEnd.x = points[0].x;
                    }
                    drawingCanvas.activePoints = [points[0], lineEnd];
                    drawingCanvas.requestPaint();
                    return;
                }
                if (editCanvas.currentTool === "rect" || editCanvas.currentTool === "ellipse") {
                    var start = points[0];
                    var shapeEnd = Qt.point(mouse.x, mouse.y);
                    if ((mouse.modifiers & Qt.ShiftModifier) !== 0) {
                        var deltaX = mouse.x - start.x;
                        var deltaY = mouse.y - start.y;
                        var side = Math.max(Math.abs(deltaX), Math.abs(deltaY));
                        shapeEnd = Qt.point(start.x + (deltaX < 0 ? -side : side),
                                           start.y + (deltaY < 0 ? -side : side));
                    }
                    drawingCanvas.activePoints = [start, shapeEnd];
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
                if (drawingCanvas.interaction === "move" || drawingCanvas.interaction === "resize"
                        || drawingCanvas.interaction === "rotate") {
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
                        width: editCanvas.thickness,
                        rotation: 0
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
                if ((drawingCanvas.interaction === "move" || drawingCanvas.interaction === "resize"
                     || drawingCanvas.interaction === "rotate")
                        && editCanvas.selectedIndex >= 0) {
                    var restored = Object.assign({}, editCanvas.strokes[editCanvas.selectedIndex],
                                                 { points: drawingCanvas.originalPoints.slice(),
                                                   rotation: drawingCanvas.originalRotation });
                    editCanvas.replaceStroke(editCanvas.selectedIndex, restored);
                }
                drawingCanvas.interaction = "";
                drawingCanvas.activePoints = [];
                drawingCanvas.requestPaint();
            }
            onExited: {
                if (!pressed && drawingCanvas.hoveredHandle !== "") {
                    drawingCanvas.hoveredHandle = "";
                    drawingCanvas.requestPaint();
                }
            }
        }
    }

    DTK.DciIcon {
        readonly property point handlePosition: {
            if (editCanvas.selectedIndex < 0
                    || editCanvas.selectedIndex >= editCanvas.strokes.length)
                return Qt.point(-100, -100);
            var handles = editCanvas.handlesForStroke(
                        editCanvas.strokes[editCanvas.selectedIndex]);
            for (var i = 0; i < handles.length; ++i) {
                if (handles[i].name === "rotate")
                    return Qt.point(handles[i].x, handles[i].y);
            }
            return Qt.point(-100, -100);
        }

        height: 20
        name: "edit_rotate"
        sourceSize.height: 20
        sourceSize.width: 20
        visible: handlePosition.x >= 0 && handlePosition.y >= 0
        width: 20
        x: handlePosition.x - width / 2
        y: handlePosition.y - height / 2
        z: 3
    }

    DTK.DciIcon {
        height: 24
        name: "edit_rotate_cursor"
        sourceSize.height: 24
        sourceSize.width: 24
        visible: drawingCanvas.hoveredHandle === "rotate"
                 || drawingCanvas.interaction === "rotate"
        width: 24
        x: canvasMouseArea.mouseX - width / 2
        y: canvasMouseArea.mouseY - height / 2
        z: 4
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
