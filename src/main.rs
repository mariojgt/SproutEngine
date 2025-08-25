//! Sprout Engine — minimal editor skeleton using Bevy + egui
//! Cross‑platform (macOS/Windows/Linux) with a tiny "visual scripting" block list.

use bevy::prelude::*;
use bevy::window::{PrimaryWindow, WindowResolution};
use bevy_egui::{egui, EguiContexts, EguiPlugin};
use std::f32::consts::PI;

// -----------------------------
// Data Model
// -----------------------------

/// Which entity is currently selected in the editor.
#[derive(Resource, Default)]
struct Selection {
    entity: Option<Entity>,
}

/// A simple visual scripting "block".
#[derive(Clone)]
enum ScriptOp {
    RotateXYZ { deg_per_sec: Vec3 },
    TranslateY { units_per_sec: f32 },
    ScalePulse { base: Vec3, amplitude: f32, speed_hz: f32 },
}

impl Default for ScriptOp {
    fn default() -> Self {
        ScriptOp::RotateXYZ { deg_per_sec: Vec3::new(0.0, 45.0, 0.0) }
    }
}

/// Component: attaches a simple script to an entity.
#[derive(Component, Default)]
struct VisualScript {
    enabled: bool,
    ops: Vec<ScriptOp>,
}

// Camera control state
#[derive(Resource, Reflect)]
#[reflect(Resource)]
struct CameraOrbit {
    target: Vec3,
    distance: f32,
    yaw: f32,
    pitch: f32,
    pan_speed: f32,
    orbit_speed: f32,
    zoom_speed: f32,
}

impl Default for CameraOrbit {
    fn default() -> Self {
        Self {
            target: Vec3::ZERO,
            distance: 6.0,
            yaw: 0.5,
            pitch: 0.25,
            pan_speed: 0.005,
            orbit_speed: 0.005,
            zoom_speed: 1.0,
        }
    }
}

// -----------------------------
// App
// -----------------------------

fn main() {
    App::new()
        .insert_resource(ClearColor(Color::srgb_u8(0x14, 0x16, 0x1a)))
        .insert_resource(Selection::default())
        .insert_resource(CameraOrbit::default())
        .add_plugins(
            DefaultPlugins
                .set(WindowPlugin {
                    primary_window: Some(Window {
                        title: "Sprout Engine".into(),
                        resolution: WindowResolution::new(1280.0, 800.0),
                        resizable: true,
                        ..default()
                    }),
                    ..default()
                })
                .set(ImagePlugin::default_nearest()), // nice crisp text
        )
        .add_plugins(EguiPlugin)
        .add_systems(Startup, setup_scene)
        .add_systems(
            Update,
            (
                ui_hierarchy,
                ui_inspector,
                ui_script_panel,
                apply_scripts,
                camera_controls,
                frame_selected_on_f_key,
            ),
        )
        .run();
}

// -----------------------------
// Startup: simple scene
// -----------------------------

fn setup_scene(mut commands: Commands, mut meshes: ResMut<Assets<Mesh>>, mut materials: ResMut<Assets<StandardMaterial>>) {
    // Ground
    commands.spawn((
        Name::new("Ground"),
        Mesh3d(meshes.add(Plane3d::default().mesh().size(20.0, 20.0))),
        MeshMaterial3d(materials.add(StandardMaterial {
            base_color: Color::srgb(0.08, 0.1, 0.12),
            perceptual_roughness: 1.0,
            metallic: 0.0,
            ..default()
        })),
        Transform::from_xyz(0.0, 0.0, 0.0),
    ));

    // A cube with a basic script
    commands.spawn((
        Name::new("Cube"),
        Mesh3d(meshes.add(Cuboid::new(1.0, 1.0, 1.0))),
        MeshMaterial3d(materials.add(StandardMaterial {
            base_color: Color::srgb(0.36, 0.72, 1.0),
            ..default()
        })),
        Transform::from_xyz(0.0, 0.5, 0.0),
        VisualScript {
            enabled: true,
            ops: vec![
                ScriptOp::RotateXYZ { deg_per_sec: Vec3::new(0.0, 45.0, 0.0) },
                ScriptOp::ScalePulse { base: Vec3::splat(1.0), amplitude: 0.15, speed_hz: 0.5 },
            ],
        },
    ));

    // Light
    commands.spawn((
        Name::new("Sun"),
        DirectionalLight {
            shadows_enabled: true,
            illuminance: 8000.0,
            ..default()
        },
        Transform::from_xyz(5.0, 10.0, 6.0).looking_at(Vec3::ZERO, Vec3::Y),
    ));

    // Camera
    commands.spawn((
        Name::new("EditorCamera"),
        Camera3d::default(),
        Transform::from_xyz(0.0, 3.0, 8.0).looking_at(Vec3::ZERO, Vec3::Y),
    ));
}

// -----------------------------
/* UI: Hierarchy (left), Inspector (right), Script (bottom) */
// -----------------------------

fn ui_hierarchy(
    mut contexts: EguiContexts,
    mut selection: ResMut<Selection>,
    query: Query<(Entity, Option<&Name>)>,
) {
    use egui::{SidePanel, ScrollArea};

    SidePanel::left("hierarchy_left")
        .default_width(220.0)
        .resizable(true)
        .show(contexts.ctx_mut(), |ui| {
            ui.heading("Hierarchy");
            ui.separator();
            ScrollArea::vertical().auto_shrink([false; 2]).show(ui, |ui| {
                for (entity, name) in query.iter() {
                    let label = name.map(|n| n.as_str().to_string()).unwrap_or_else(|| format!("Entity {:?}", entity));
                    let selected = selection.entity == Some(entity);
                    let btn = ui.selectable_label(selected, label);
                    if btn.clicked() {
                        selection.entity = Some(entity);
                    }
                }
            });
        });
}

fn ui_inspector(
    mut contexts: EguiContexts,
    selection: Res<Selection>,
    mut transforms: Query<&mut Transform>,
    names: Query<&Name>,
) {
    use egui::SidePanel;
    SidePanel::right("inspector_right")
        .default_width(300.0)
        .resizable(true)
        .show(contexts.ctx_mut(), |ui| {
            ui.heading("Inspector");
            ui.separator();
            if let Some(entity) = selection.entity {
                if let Ok(name) = names.get(entity) {
                    ui.label(format!("Selected: {}", name));
                }

                if let Ok(mut t) = transforms.get_mut(entity) {
                    ui.separator();
                    ui.label("Transform");

                    // Translation
                    let mut pos = t.translation.to_array();
                    if ui.add(egui::DragValue::new(&mut pos[0]).speed(0.05)).changed() ||
                       ui.add(egui::DragValue::new(&mut pos[1]).speed(0.05)).changed() ||
                       ui.add(egui::DragValue::new(&mut pos[2]).speed(0.05)).changed() {
                        t.translation = Vec3::new(pos[0], pos[1], pos[2]);
                    }
                    ui.horizontal(|ui| { ui.label("Position"); ui.monospace(format!("{:.2}, {:.2}, {:.2}", pos[0], pos[1], pos[2])); });

                    // Rotation (Euler degrees)
                    let (mut pitch, mut yaw, mut roll) = t.rotation.to_euler(EulerRot::XYZ);
                    let mut deg = [pitch.to_degrees(), yaw.to_degrees(), roll.to_degrees()];
                    let mut changed = false;
                    changed |= ui.add(egui::DragValue::new(&mut deg[0]).speed(0.2)).changed();
                    changed |= ui.add(egui::DragValue::new(&mut deg[1]).speed(0.2)).changed();
                    changed |= ui.add(egui::DragValue::new(&mut deg[2]).speed(0.2)).changed();
                    if changed {
                        pitch = deg[0].to_radians();
                        yaw   = deg[1].to_radians();
                        roll  = deg[2].to_radians();
                        t.rotation = Quat::from_euler(EulerRot::XYZ, pitch, yaw, roll);
                    }
                    ui.horizontal(|ui| { ui.label("Rotation°"); ui.monospace(format!("{:.1}, {:.1}, {:.1}", deg[0], deg[1], deg[2])); });

                    // Scale
                    let mut scl = t.scale.to_array();
                    if ui.add(egui::DragValue::new(&mut scl[0]).speed(0.02)).changed() ||
                       ui.add(egui::DragValue::new(&mut scl[1]).speed(0.02)).changed() ||
                       ui.add(egui::DragValue::new(&mut scl[2]).speed(0.02)).changed() {
                        t.scale = Vec3::new(scl[0].max(0.001), scl[1].max(0.001), scl[2].max(0.001));
                    }
                    ui.horizontal(|ui| { ui.label("Scale"); ui.monospace(format!("{:.2}, {:.2}, {:.2}", scl[0], scl[1], scl[2])); });
                } else {
                    ui.label("No transform found.");
                }
            } else {
                ui.label("Nothing selected");
            }
        });
}

fn ui_script_panel(
    mut contexts: EguiContexts,
    selection: Res<Selection>,
    mut query: Query<&mut VisualScript>,
) {
    use egui::TopBottomPanel;
    TopBottomPanel::bottom("visual_script_bottom")
        .resizable(true)
        .default_height(180.0)
        .show(contexts.ctx_mut(), |ui| {
            ui.heading("Visual Script");
            ui.separator();
            if let Some(e) = selection.entity {
                if let Ok(mut script) = query.get_mut(e) {
                    ui.horizontal(|ui| {
                        ui.checkbox(&mut script.enabled, "Enabled");
                        if ui.button("Add: Rotate").clicked() {
                            script.ops.push(ScriptOp::RotateXYZ { deg_per_sec: Vec3::new(0.0, 45.0, 0.0) });
                        }
                        if ui.button("Add: Translate Y").clicked() {
                            script.ops.push(ScriptOp::TranslateY { units_per_sec: 0.5 });
                        }
                        if ui.button("Add: Scale Pulse").clicked() {
                            script.ops.push(ScriptOp::ScalePulse { base: Vec3::splat(1.0), amplitude: 0.15, speed_hz: 0.5 });
                        }
                    });
                    ui.separator();

                    // Show ops list
                    let mut remove_idx: Option<usize> = None;
                    let mut swap_down_idx: Option<usize> = None;
                    let mut swap_up_idx: Option<usize> = None;

                    let ops_len = script.ops.len();
                    for i in 0..ops_len {
                        ui.group(|ui| {
                            ui.horizontal(|ui| {
                                ui.label(format!("Step {}", i + 1));
                                if ui.small_button("▲").clicked() && i > 0 {
                                    swap_up_idx = Some(i);
                                }
                                if ui.small_button("▼").clicked() && i + 1 < ops_len {
                                    swap_down_idx = Some(i);
                                }
                                if ui.small_button("✖").clicked() {
                                    remove_idx = Some(i);
                                }
                            });

                            let op = &mut script.ops[i];
                            match op {
                                ScriptOp::RotateXYZ { deg_per_sec } => {
                                    ui.label("RotateXYZ (deg/s)");
                                    ui.horizontal(|ui| {
                                        ui.add(egui::DragValue::new(&mut deg_per_sec.x).speed(1.0));
                                        ui.add(egui::DragValue::new(&mut deg_per_sec.y).speed(1.0));
                                        ui.add(egui::DragValue::new(&mut deg_per_sec.z).speed(1.0));
                                    });
                                }
                                ScriptOp::TranslateY { units_per_sec } => {
                                    ui.label("TranslateY (units/s)");
                                    ui.add(egui::DragValue::new(units_per_sec).speed(0.05));
                                }
                                ScriptOp::ScalePulse { base, amplitude, speed_hz } => {
                                    ui.label("ScalePulse");
                                    ui.horizontal(|ui| {
                                        ui.label("Base");
                                        ui.add(egui::DragValue::new(&mut base.x).speed(0.02));
                                        ui.add(egui::DragValue::new(&mut base.y).speed(0.02));
                                        ui.add(egui::DragValue::new(&mut base.z).speed(0.02));
                                    });
                                    ui.horizontal(|ui| {
                                        ui.label("Amplitude");
                                        ui.add(egui::DragValue::new(amplitude).speed(0.01));
                                        ui.label("Speed (Hz)");
                                        ui.add(egui::DragValue::new(speed_hz).speed(0.01));
                                    });
                                }
                            }
                        });
                    }

                    // Apply operations after iteration
                    if let Some(idx) = remove_idx {
                        script.ops.remove(idx);
                    }
                    if let Some(idx) = swap_up_idx {
                        script.ops.swap(idx, idx - 1);
                    }
                    if let Some(idx) = swap_down_idx {
                        script.ops.swap(idx, idx + 1);
                    }
                } else {
                    ui.label("Selected entity has no VisualScript component. Click 'Add Script' below to attach one.");
                    if ui.button("Add Script").clicked() {
                        // Can't mutate components of an entity we can't query here; recommend using context menu in hierarchy in a real editor.
                    }
                }
            } else {
                ui.label("Nothing selected");
            }
        });
}

// -----------------------------
// Scripting & Systems
// -----------------------------

fn apply_scripts(
    time: Res<Time>,
    mut q: Query<(&mut Transform, &VisualScript)>,
) {
    for (mut t, script) in q.iter_mut() {
        if !script.enabled { continue; }

        for op in &script.ops {
            match *op {
                ScriptOp::RotateXYZ { deg_per_sec } => {
                    let rad = Vec3::new(
                        deg_per_sec.x.to_radians(),
                        deg_per_sec.y.to_radians(),
                        deg_per_sec.z.to_radians(),
                    ) * time.delta_secs();
                    let qx = Quat::from_rotation_x(rad.x);
                    let qy = Quat::from_rotation_y(rad.y);
                    let qz = Quat::from_rotation_z(rad.z);
                    t.rotation = qy * qx * qz * t.rotation;
                }
                ScriptOp::TranslateY { units_per_sec } => {
                    t.translation.y += units_per_sec * time.delta_secs();
                }
                ScriptOp::ScalePulse { base, amplitude, speed_hz } => {
                    let s = base + Vec3::splat(amplitude) * (2.0 * PI * speed_hz * time.elapsed_secs()).sin();
                    t.scale = s.max(Vec3::splat(0.001));
                }
            }
        }
    }
}

// -----------------------------
// Camera Controls (orbit / pan / zoom)
// -----------------------------

fn camera_controls(
    mut windows: Query<&mut Window, With<PrimaryWindow>>,
    mut camera_q: Query<&mut Transform, (With<Camera3d>, Without<DirectionalLight>)>,
    mut orbit: ResMut<CameraOrbit>,
    mouse: Res<ButtonInput<MouseButton>>,
    keys: Res<ButtonInput<KeyCode>>,
    mut motion_evr: EventReader<bevy::input::mouse::MouseMotion>,
    mut wheel_evr: EventReader<bevy::input::mouse::MouseWheel>,
) {
    let mut cam = if let Ok(c) = camera_q.get_single_mut() { c } else { return; };

    let mut delta = Vec2::ZERO;
    for ev in motion_evr.read() {
        delta += ev.delta;
    }

    let mut zoom = 0.0f32;
    for ev in wheel_evr.read() {
        zoom += ev.y;
    }
    if zoom.abs() > 0.0 {
        orbit.distance = (orbit.distance * (1.0 - zoom * 0.1 * orbit.zoom_speed)).clamp(0.5, 100.0);
    }

    // Orbit (RMB)
    if mouse.pressed(MouseButton::Right) && delta.length_squared() > 0.0 {
        orbit.yaw   -= delta.x * orbit.orbit_speed;
        orbit.pitch -= delta.y * orbit.orbit_speed;
        orbit.pitch = orbit.pitch.clamp(-1.54, 1.54);
    }

    // Pan (MMB or Shift+RMB)
    if mouse.pressed(MouseButton::Middle) || (mouse.pressed(MouseButton::Right) && keys.any_pressed([KeyCode::ShiftLeft, KeyCode::ShiftRight])) {
        // Pan relative to camera axes
        let right = cam.right();
        let up    = cam.up();
        let pan_speed = orbit.pan_speed;
        let distance = orbit.distance;
        orbit.target += (-right * delta.x + up * delta.y) * pan_speed * distance.max(1.0);
    }

    // Update camera transform from orbit params
    let rot = Quat::from_euler(EulerRot::ZYX, 0.0, orbit.yaw, orbit.pitch);
    cam.translation = orbit.target + rot * (Vec3::Z * orbit.distance);
    cam.look_at(orbit.target, Vec3::Y);

    // Confine cursor to window on RMB drag (nice UX but optional)
    if let Ok(mut window) = windows.get_single_mut() {
        window.cursor_options.visible = !mouse.pressed(MouseButton::Right);
    }
}

fn frame_selected_on_f_key(
    selection: Res<Selection>,
    keys: Res<ButtonInput<KeyCode>>,
    mut orbit: ResMut<CameraOrbit>,
    transforms: Query<&Transform>,
) {
    if keys.just_pressed(KeyCode::KeyF) {
        if let Some(e) = selection.entity {
            if let Ok(t) = transforms.get(e) {
                orbit.target = t.translation;
            }
        }
    }
}
