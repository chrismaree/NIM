import adsk.core
import adsk.fusion
import traceback


def mm(value_mm: float) -> float:
    """Fusion API createByReal uses centimeters for length."""
    return value_mm / 10.0


def add_center_rect(sketch: adsk.fusion.Sketch, width_mm: float, height_mm: float):
    lines = sketch.sketchCurves.sketchLines
    p1 = adsk.core.Point3D.create(-mm(width_mm / 2.0), -mm(height_mm / 2.0), 0)
    p2 = adsk.core.Point3D.create(mm(width_mm / 2.0), mm(height_mm / 2.0), 0)
    lines.addTwoPointRectangle(p1, p2)


def add_circle(sketch: adsk.fusion.Sketch, x_mm: float, y_mm: float, diameter_mm: float):
    circles = sketch.sketchCurves.sketchCircles
    center = adsk.core.Point3D.create(mm(x_mm), mm(y_mm), 0)
    circles.addByCenterRadius(center, mm(diameter_mm / 2.0))


def add_text_block(
    sketch: adsk.fusion.Sketch,
    text: str,
    x_mm: float,
    y_mm: float,
    box_w_mm: float,
    box_h_mm: float,
    text_h_mm: float,
):
    texts = sketch.sketchTexts
    txt_input = texts.createInput2(text, mm(text_h_mm))
    corner = adsk.core.Point3D.create(mm(x_mm), mm(y_mm), 0)
    diagonal = adsk.core.Point3D.create(mm(x_mm + box_w_mm), mm(y_mm + box_h_mm), 0)
    txt_input.setAsMultiLine(
        corner,
        diagonal,
        adsk.core.HorizontalAlignments.LeftHorizontalAlignment,
        adsk.core.VerticalAlignments.TopVerticalAlignment,
        0,
    )
    return texts.add(txt_input)


def extrude_profile(
    extrudes: adsk.fusion.ExtrudeFeatures,
    profile,
    operation: adsk.fusion.FeatureOperations,
    distance_mm: float,
):
    ext_input = extrudes.createInput(profile, operation)
    ext_input.setDistanceExtent(False, adsk.core.ValueInput.createByReal(mm(distance_mm)))
    return extrudes.add(ext_input)


def extrude_text(
    extrudes: adsk.fusion.ExtrudeFeatures,
    sketch_text: adsk.fusion.SketchText,
    operation: adsk.fusion.FeatureOperations,
    distance_mm: float,
):
    ext_input = extrudes.createInput(sketch_text, operation)
    ext_input.setDistanceExtent(False, adsk.core.ValueInput.createByReal(mm(distance_mm)))
    return extrudes.add(ext_input)


def run(context):
    ui = None
    try:
        app = adsk.core.Application.get()
        ui = app.userInterface

        # ----- Dimensions (mm) -----
        outer_x = 230.0
        outer_y = 160.0
        outer_z = 50.0

        wall = 3.0
        floor = 3.0
        top = 3.0

        # Nim control geometry
        row_lengths = [1, 3, 5, 7]
        row_pitch = 24.0
        pair_pitch = 28.0
        pair_dx = 10.0  # LED to the right of each button
        first_row_y = 45.0

        turn_led_y = 58.0
        turn_led_x = 96.0

        start_btn_x = 0.0
        start_btn_y = -58.0

        # ----- New design -----
        app.documents.add(adsk.core.DocumentTypes.FusionDesignDocumentType)
        design = adsk.fusion.Design.cast(app.activeProduct)
        root = design.rootComponent

        sketches = root.sketches
        extrudes = root.features.extrudeFeatures
        planes = root.constructionPlanes

        # ----- Outer box -----
        base_sketch = sketches.add(root.xYConstructionPlane)
        add_center_rect(base_sketch, outer_x, outer_y)
        outer_profile = base_sketch.profiles.item(0)
        outer_extrude = extrude_profile(
            extrudes,
            outer_profile,
            adsk.fusion.FeatureOperations.NewBodyFeatureOperation,
            outer_z,
        )
        body = outer_extrude.bodies.item(0)
        body.name = "NIM_Box"

        # ----- Internal cavity (keeps top and floor thicknesses) -----
        cavity_plane_input = planes.createInput()
        cavity_plane_input.setByOffset(root.xYConstructionPlane, adsk.core.ValueInput.createByReal(mm(floor)))
        cavity_plane = planes.add(cavity_plane_input)

        cavity_sketch = sketches.add(cavity_plane)
        add_center_rect(cavity_sketch, outer_x - 2 * wall, outer_y - 2 * wall)
        cavity_profile = cavity_sketch.profiles.item(0)
        cavity_depth = outer_z - floor - top
        extrude_profile(
            extrudes,
            cavity_profile,
            adsk.fusion.FeatureOperations.CutFeatureOperation,
            cavity_depth,
        )

        # ----- Bottom service opening (for assembly and wiring access) -----
        bottom_open_sketch = sketches.add(root.xYConstructionPlane)
        add_center_rect(bottom_open_sketch, outer_x - 28.0, outer_y - 28.0)
        bottom_open_profile = bottom_open_sketch.profiles.item(0)
        extrude_profile(
            extrudes,
            bottom_open_profile,
            adsk.fusion.FeatureOperations.CutFeatureOperation,
            floor + 1.0,
        )

        # ----- Hole sketch plane inside top thickness -----
        hole_plane_input = planes.createInput()
        hole_plane_input.setByOffset(
            root.xYConstructionPlane,
            adsk.core.ValueInput.createByReal(mm(outer_z - top + 0.2)),
        )
        hole_plane = planes.add(hole_plane_input)
        hole_sketch = sketches.add(hole_plane)

        # Token holes in 1-3-5-7 layout:
        # button holes = 7mm, LED holes = 3mm
        for row_index, row_count in enumerate(row_lengths):
            y = first_row_y - (row_index * row_pitch)
            start_x = -((row_count - 1) * pair_pitch) / 2.0
            for c in range(row_count):
                x_button = start_x + (c * pair_pitch)
                x_led = x_button + pair_dx
                add_circle(hole_sketch, x_button, y, 7.0)
                add_circle(hole_sketch, x_led, y, 3.0)

        # Two turn LEDs (18 total LED holes = 16 token LEDs + 2 turn LEDs)
        add_circle(hole_sketch, -turn_led_x, turn_led_y, 3.0)
        add_circle(hole_sketch, turn_led_x, turn_led_y, 3.0)

        # Start/reset button
        add_circle(hole_sketch, start_btn_x, start_btn_y, 7.0)

        hole_profiles = adsk.core.ObjectCollection.create()
        for i in range(hole_sketch.profiles.count):
            hole_profiles.add(hole_sketch.profiles.item(i))

        extrude_profile(
            extrudes,
            hole_profiles,
            adsk.fusion.FeatureOperations.CutFeatureOperation,
            top + 1.5,
        )

        # ----- Top text labels -----
        top_text_plane_input = planes.createInput()
        top_text_plane_input.setByOffset(
            root.xYConstructionPlane, adsk.core.ValueInput.createByReal(mm(outer_z))
        )
        top_text_plane = planes.add(top_text_plane_input)
        top_text_sketch = sketches.add(top_text_plane)

        p1_txt = add_text_block(
            top_text_sketch, "PLAYER 1", -112.0, 66.0, 56.0, 10.0, 5.0
        )
        p2_txt = add_text_block(
            top_text_sketch, "PLAYER 2", 56.0, 66.0, 56.0, 10.0, 5.0
        )

        extrude_text(
            extrudes,
            p1_txt,
            adsk.fusion.FeatureOperations.JoinFeatureOperation,
            0.8,
        )
        extrude_text(
            extrudes,
            p2_txt,
            adsk.fusion.FeatureOperations.JoinFeatureOperation,
            0.8,
        )

        # ----- Side text "NIM" (raised) -----
        side_text_plane_input = planes.createInput()
        side_text_plane_input.setByOffset(
            root.yZConstructionPlane,
            adsk.core.ValueInput.createByReal(mm((outer_x / 2.0) - 0.2)),
        )
        side_text_plane = planes.add(side_text_plane_input)
        side_text_sketch = sketches.add(side_text_plane)
        nim_txt = add_text_block(side_text_sketch, "NIM", -26.0, 18.0, 52.0, 18.0, 12.0)
        extrude_text(
            extrudes,
            nim_txt,
            adsk.fusion.FeatureOperations.JoinFeatureOperation,
            1.0,
        )

        ui.messageBox(
            "NIM enclosure generated.\n"
            "Size: 230 x 160 x 50 mm\n"
            "Token layout: 1-3-5-7\n"
            "Holes: 7mm buttons, 3mm LEDs\n"
            "Includes PLAYER 1 / PLAYER 2 and NIM text."
        )

    except Exception:
        if ui:
            ui.messageBox("Failed:\n{}".format(traceback.format_exc()))

