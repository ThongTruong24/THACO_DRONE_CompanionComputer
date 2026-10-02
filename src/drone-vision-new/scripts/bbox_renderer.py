from dataclasses import dataclass

@dataclass(frozen=True)
class Style:
    bbox: tuple
    label: str
    color: tuple
    thickness: int

def render_plan(detections, selected):
    return [Style(d.bbox, f'{d.label} {d.confidence:.2f}' + (' [SELECTED]' if d is selected else ''),
                  (0, 255, 255) if d is selected else (0, 255, 0), 4 if d is selected else 2)
            for d in detections]

def render(frame, detections, selected):
    import cv2
    annotated = frame.copy()
    height, width = frame.shape[:2]
    for style in render_plan(detections, selected):
        x1, y1, x2, y2 = [int(v) for v in style.bbox]
        x1, x2 = [max(0, min(width - 1, v)) for v in (x1, x2)]
        y1, y2 = [max(0, min(height - 1, v)) for v in (y1, y2)]
        cv2.rectangle(annotated, (x1, y1), (x2, y2), style.color, style.thickness)
        cv2.putText(annotated, style.label, (x1, max(18, y1 - 8)),
                    cv2.FONT_HERSHEY_SIMPLEX, .55, style.color, 2, cv2.LINE_AA)
    return annotated
