//! Sprout Engine — Unified
//! Minimal editor with save/load, node graph, content browser (glTF/images), gizmos, and click selection.

use bevy::prelude::*;
use bevy::window::{PrimaryWindow, WindowResolution};
use bevy::render::camera::Camera;
use bevy::input::{ButtonInput, mouse::MouseMotion, mouse::MouseWheel};
use bevy_egui::{egui, EguiContexts, EguiPlugin};
use serde::{Serialize, Deserialize};
use std::fs;
use std::path::Path;

fn main() {
    App::new()
        .insert_resource(ClearColor(Color::srgb(0.078, 0.086, 0.102)))
        .insert_resource(EditorState::default())
        .insert_resource(Selection::default())
        .insert_resource(CameraOrbit::default())
        .add_plugins(
            DefaultPlugins
                .set(WindowPlugin {
                    primary_window: Some(Window {
                        title: "Sprout Engine".into(),
                        resolution: WindowResolution::new(1440.0, 900.0),
                        resizable: true,
                        ..default()
                    }),
                    ..default()
                })
                .set(AssetPlugin {
                    file_path: "assets".into(),
                    ..default()
                })
                .set(ImagePlugin::default_nearest()),
        )
        .add_plugins(EguiPlugin)
        .add_systems(Startup, setup_scene)
        .add_systems(
            Update,
            (
                ui_menu,
                ui_hierarchy,
                ui_inspector,
                ui_script_panel,
                ui_node_graph,
                ui_content_browser,
                apply_scripts,
                camera_controls,
                frame_selected_on_f_key,
                draw_gizmos,
                handle_gizmo_drag,
                picking_by_click,
            ),
        )
        .run();
}

/* =====================
    Data & Components
   ===================== */

#[derive(Resource, Default)]
struct Selection {
    entity: Option<Entity>,
}

#[derive(Clone, Serialize, Deserialize)]
enum ScriptOp {
    RotateXYZ { deg_per_sec: [f32; 3] },
    TranslateY { units_per_sec: f32 },
    ScalePulse { base: [f32; 3], amplitude: f32, speed_hz: f32 },
}

impl Default for ScriptOp {
    fn default() -> Self {
        ScriptOp::RotateXYZ { deg_per_sec: [0.0, 45.0, 0.0] }
    }
}

#[derive(Component, Default, Serialize, Deserialize, Clone)]
struct VisualScript {
    enabled: bool,
    ops: Vec<ScriptOp>,
}

/* Node-graph (very simple) */

#[derive(Clone, Serialize, Deserialize)]
struct Node {
    id: u64,
    title: String,
    pos: [f32; 2],
    op: ScriptOp,
}

#[derive(Component, Default, Serialize, Deserialize, Clone)]
struct NodeGraph {
    nodes: Vec<Node>,
    links: Vec<(u64, u64)>,
    next_id: u64,
}

#[derive(Component, Clone, Serialize, Deserialize)]
enum SpawnKind {
    Cube,
    Gltf { path: String, scale: f32 },
    ImageQuad { path: String, size: f32 },
}

/* Scene serialization */

#[derive(Serialize, Deserialize, Default)]
struct SceneFile {
    entities: Vec<SceneEntity>,
}

#[derive(Serialize, Deserialize)]
struct SceneEntity {
    name: String,
    translation: [f32; 3],
    rotation_euler_deg: [f32; 3],
    scale: [f32; 3],
    kind: SpawnKind,
    script: Option<VisualScript>,
    graph: Option<NodeGraph>,
}

#[derive(Resource)]
struct EditorState {
    file_path: String,
    gizmo: GizmoMode,
    axis_lock: AxisLock,
    dragging: bool,
    last_mouse: Vec2,
    content_files: Vec<String>,
    selected_file_idx: Option<usize>,
}

impl Default for EditorState {
    fn default() -> Self {
        Self {
            file_path: "sprout_scene.json".into(),
            gizmo: GizmoMode::Translate,
            axis_lock: AxisLock::None,
            dragging: false,
            last_mouse: Vec2::ZERO,
            content_files: vec![],
            selected_file_idx: None,
        }
    }
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum GizmoMode { Translate, Rotate, Scale }

#[derive(Clone, Copy, PartialEq, Eq)]
enum AxisLock { None, X, Y, Z }

/* Orbit camera */

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

/* =====================
   Startup: demo scene
   ===================== */

fn setup_scene(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    // Ground
    commands.spawn((
        Name::new("Ground"),
        Mesh3d(meshes.add(Plane3d::default().mesh().size(20.0, 20.0))),
        MeshMaterial3d(materials.add(StandardMaterial {
            base_color: Color::srgb(0.08, 0.1, 0.12),
            perceptual_roughness: 1.0,
            ..default()
        })),
        Transform::default(),
        SpawnKind::Cube, // treat ground as cube kind for save simplicity
        PickBounds { radius: 100.0 },
    ));

    // Cube with script and node graph
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
                ScriptOp::RotateXYZ { deg_per_sec: [0.0, 45.0, 0.0] },
                ScriptOp::ScalePulse { base: [1.0, 1.0, 1.0], amplitude: 0.15, speed_hz: 0.5 },
            ],
        },
        NodeGraph {
            nodes: vec![
                Node { id: 1, title: "Rotate".into(), pos: [20.0, 20.0], op: ScriptOp::RotateXYZ { deg_per_sec: [0.0, 45.0, 0.0] } },
                Node { id: 2, title: "ScalePulse".into(), pos: [220.0, 120.0], op: ScriptOp::ScalePulse { base: [1.0,1.0,1.0], amplitude: 0.15, speed_hz: 0.5 } },
            ],
            links: vec![(1,2)],
            next_id: 3,
        },
        SpawnKind::Cube,
        PickBounds { radius: 0.9 },
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

/* =====================
   UI: Menu + Panels
   ===================== */

fn ui_menu(
    mut contexts: EguiContexts,
    mut state: ResMut<EditorState>,
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    q_entities: Query<(Entity, &Name)>,
    q_full: Query<(Entity, &Name, &Transform, Option<&VisualScript>, Option<&NodeGraph>, Option<&SpawnKind>)>,
) {
    egui::TopBottomPanel::top("menu_top").show(contexts.ctx_mut(), |ui| {
        egui::menu::bar(ui, |ui| {
            ui.menu_button("File", |ui| {
                if ui.button("New Scene").clicked() {
                    // Despawn everything except camera & light
                    for (e, name) in q_entities.iter() {
                        let n = name.as_str();
                        if n != "EditorCamera" && n != "Sun" {
                            commands.entity(e).despawn_recursive();
                        }
                    }
                    // Add one cube
                    commands.spawn((
                        Name::new("Cube"),
                        Mesh3d(meshes.add(Cuboid::new(1.0, 1.0, 1.0))),
                        MeshMaterial3d(materials.add(StandardMaterial { base_color: Color::srgb(0.36, 0.72, 1.0), ..default() })),
                        Transform::from_xyz(0.0, 0.5, 0.0),
                        VisualScript { enabled: true, ops: vec![ScriptOp::RotateXYZ { deg_per_sec: [0.0, 45.0, 0.0] } ] },
                        NodeGraph::default(),
                        SpawnKind::Cube,
                        PickBounds { radius: 0.9 },
                    ));
                    ui.close_menu();
                }
                ui.separator();
                ui.horizontal(|ui| {
                    ui.label("Path:");
                    ui.text_edit_singleline(&mut state.file_path);
                });
                if ui.button("Save").clicked() {
                    let mut file = SceneFile { entities: vec![] };
                    for (_, name, t, script, graph, kind) in q_full.iter() {
                        let n = name.as_str();
                        if n == "EditorCamera" || n == "Sun" || n == "Ground" { continue; }
                        let (pitch, yaw, roll) = t.rotation.to_euler(EulerRot::XYZ);
                        file.entities.push(SceneEntity {
                            name: name.to_string(),
                            translation: t.translation.to_array(),
                            rotation_euler_deg: [pitch.to_degrees(), yaw.to_degrees(), roll.to_degrees()],
                            scale: t.scale.to_array(),
                            kind: kind.cloned().unwrap_or(SpawnKind::Cube),
                            script: script.cloned(),
                            graph: graph.cloned(),
                        });
                    }
                    if let Ok(json) = serde_json::to_string_pretty(&file) {
                        let _ = fs::write(&state.file_path, json);
                    }
                    ui.close_menu();
                }
                if ui.button("Load").clicked() {
                    if let Ok(bytes) = fs::read(&state.file_path) {
                        if let Ok(scene) = serde_json::from_slice::<SceneFile>(&bytes) {
                            // Clear scene (except camera + light + ground)
                            for (e, name) in q_entities.iter() {
                                let n = name.as_str();
                                if n != "EditorCamera" && n != "Sun" && n != "Ground" {
                                    commands.entity(e).despawn_recursive();
                                }
                            }
                            // Rebuild
                            for ent in scene.entities {
                                spawn_from_kind(&mut commands, &mut meshes, &mut materials, ent);
                            }
                        }
                    }
                    ui.close_menu();
                }
            });

            ui.menu_button("Gizmo", |ui| {
                ui.horizontal(|ui| {
                    if ui.selectable_label(matches!(state.gizmo, GizmoMode::Translate), "W Move").clicked() { state.gizmo = GizmoMode::Translate; }
                    if ui.selectable_label(matches!(state.gizmo, GizmoMode::Rotate),   "E Rotate").clicked() { state.gizmo = GizmoMode::Rotate; }
                    if ui.selectable_label(matches!(state.gizmo, GizmoMode::Scale),    "R Scale").clicked() { state.gizmo = GizmoMode::Scale; }
                });
                ui.separator();
                ui.horizontal(|ui| {
                    if ui.selectable_label(matches!(state.axis_lock, AxisLock::None), "Free").clicked() { state.axis_lock = AxisLock::None; }
                    if ui.selectable_label(matches!(state.axis_lock, AxisLock::X), "X").clicked() { state.axis_lock = AxisLock::X; }
                    if ui.selectable_label(matches!(state.axis_lock, AxisLock::Y), "Y").clicked() { state.axis_lock = AxisLock::Y; }
                    if ui.selectable_label(matches!(state.axis_lock, AxisLock::Z), "Z").clicked() { state.axis_lock = AxisLock::Z; }
                });
            });
        });
    });
}

fn spawn_from_kind(
    commands: &mut Commands,
    meshes: &mut ResMut<Assets<Mesh>>,
    materials: &mut ResMut<Assets<StandardMaterial>>,
    ent: SceneEntity,
) {
    let mut transform = Transform {
        translation: Vec3::from_array(ent.translation),
        rotation: Quat::from_euler(
            EulerRot::XYZ,
            ent.rotation_euler_deg[0].to_radians(),
            ent.rotation_euler_deg[1].to_radians(),
            ent.rotation_euler_deg[2].to_radians(),
        ),
        scale: Vec3::from_array(ent.scale),
    };

    match ent.kind.clone() {
        SpawnKind::Cube => {
            let mut e = commands.spawn((
                Name::new(ent.name),
                Mesh3d(meshes.add(Cuboid::new(1.0, 1.0, 1.0))),
                MeshMaterial3d(materials.add(StandardMaterial { base_color: Color::srgb(0.36, 0.72, 1.0), ..default() })),
                transform,
                SpawnKind::Cube,
                PickBounds { radius: 0.9 },
            ));
            if let Some(s) = ent.script { e.insert(s); }
            if let Some(g) = ent.graph { e.insert(g); }
        }
        SpawnKind::Gltf { path, scale } => {
            // Workaround: spawn SceneBundle; the loader resolves by path relative to assets/
            let mut e = commands.spawn((
                Name::new(ent.name),
                SceneRoot(asset_path_to_handle_scene(&path)),
                Transform { translation: transform.translation, rotation: transform.rotation, scale: transform.scale * scale },
                SpawnKind::Gltf { path: path.clone(), scale },
                PickBounds { radius: 1.5 * scale },
            ));
            if let Some(s) = ent.script { e.insert(s); }
            if let Some(g) = ent.graph { e.insert(g); }
        }
        SpawnKind::ImageQuad { path, size } => {
            let tex_handle: Handle<Image> = asset_path_to_handle_image(&path);
            let mut e = commands.spawn((
                Name::new(ent.name),
                Mesh3d(meshes.add(Rectangle::new(size, size))),
                MeshMaterial3d(materials.add(StandardMaterial {
                    base_color_texture: Some(tex_handle),
                    unlit: true,
                    ..default()
                })),
                transform,
                SpawnKind::ImageQuad { path: path.clone(), size },
                PickBounds { radius: size.max(0.5) },
            ));
            if let Some(s) = ent.script { e.insert(s); }
            if let Some(g) = ent.graph { e.insert(g); }
        }
    }
}

/* =====================
   Content Browser
   ===================== */

fn scan_assets() -> Vec<String> {
    let mut out = vec![];
    if let Ok(rd) = fs::read_dir("assets") {
        for e in rd.flatten() {
            if let Some(ext) = e.path().extension().and_then(|s| s.to_str()) {
                let ext = ext.to_ascii_lowercase();
                if ["gltf","glb","png","jpg","jpeg"].contains(&ext.as_str()) {
                    if let Some(p) = e.path().to_str() { out.push(p.replace('\\', "/").replacen("assets/", "", 1)); }
                }
            }
        }
    }
    out.sort();
    out
}

fn ui_content_browser(
    mut contexts: EguiContexts,
    mut state: ResMut<EditorState>,
    mut commands: Commands,
    asset_server: Res<AssetServer>,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    egui::TopBottomPanel::bottom("content_browser")
        .default_height(180.0)
        .resizable(true)
        .show(contexts.ctx_mut(), |ui| {
            ui.horizontal(|ui| {
                if ui.button("Refresh").clicked() {
                    state.content_files = scan_assets();
                }
                if state.content_files.is_empty() {
                    state.content_files = scan_assets();
                }
                ui.separator();
                ui.label("assets/");
            });
            ui.separator();

            egui::ScrollArea::horizontal().show(ui, |ui| {
                for (i, p) in state.content_files.iter().enumerate() {
                    let selected = state.selected_file_idx == Some(i);
                    if ui.selectable_label(selected, p).clicked() {
                        state.selected_file_idx = Some(i);
                    }
                }
            });

            ui.separator();
            if let Some(i) = state.selected_file_idx {
                let path = state.content_files[i].clone();
                ui.horizontal(|ui| {
                    ui.label(format!("Selected: {}", path));
                    if ui.button("Spawn").clicked() {
                        let ext = Path::new(&path).extension().and_then(|s| s.to_str()).unwrap_or("").to_ascii_lowercase();
                        if ext == "gltf" || ext == "glb" {
                            commands.spawn((
                                Name::new(Path::new(&path).file_stem().unwrap_or_default().to_string_lossy().to_string()),
                                SceneRoot(asset_server.load::<Scene>(path.as_str())),
                                Transform::from_xyz(0.0, 0.0, 0.0),
                                SpawnKind::Gltf { path: path.clone(), scale: 1.0 },
                                PickBounds { radius: 1.5 },
                            ));
                        } else if ["png","jpg","jpeg"].contains(&ext.as_str()) {
                            let tex_handle: Handle<Image> = asset_server.load(path.as_str());
                            commands.spawn((
                                Name::new(Path::new(&path).file_stem().unwrap_or_default().to_string_lossy().to_string()),
                                Mesh3d(meshes.add(Rectangle::new(1.0, 1.0))),
                                MeshMaterial3d(materials.add(StandardMaterial {
                                    base_color_texture: Some(tex_handle),
                                    unlit: true,
                                    ..default()
                                })),
                                Transform::from_xyz(0.0, 0.5, 0.0),
                                SpawnKind::ImageQuad { path: path.clone(), size: 1.0 },
                                PickBounds { radius: 1.0 },
                            ));
                        }
                    }
                });
            } else {
                ui.label("Select a file to spawn it into the scene.");
            }
        });
}

/* Asset server helpers */

fn asset_path_to_handle_scene(path: &str) -> Handle<Scene> {
    // relative to assets/
    Handle::<Scene>::default() // Simplified for now
}

fn asset_path_to_handle_image(path: &str) -> Handle<Image> {
    Handle::<Image>::default() // Simplified for now
}

/* =====================
   Visual Script UI
   ===================== */

fn ui_hierarchy(
    mut contexts: EguiContexts,
    mut selection: ResMut<Selection>,
    query: Query<(Entity, Option<&Name>)>,
) {
    egui::SidePanel::left("hierarchy_left")
        .default_width(220.0)
        .resizable(true)
        .show(contexts.ctx_mut(), |ui| {
            ui.heading("Hierarchy");
            ui.separator();
            egui::ScrollArea::vertical().auto_shrink([false; 2]).show(ui, |ui| {
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
    mut state: ResMut<EditorState>,
    keys: Res<ButtonInput<KeyCode>>,
) {
    egui::SidePanel::right("inspector_right")
        .default_width(320.0)
        .resizable(true)
        .show(contexts.ctx_mut(), |ui| {
            ui.heading("Inspector");
            ui.separator();
            ui.label("Shortcuts: W/E/R (Move/Rotate/Scale), X/Y/Z axis lock, F frame");
            if let Some(entity) = selection.entity {
                if let Ok(name) = names.get(entity) {
                    ui.label(format!("Selected: {}", name));
                }
                if let Ok(mut t) = transforms.get_mut(entity) {
                    // Mode shortcuts
                    if keys.just_pressed(KeyCode::KeyW) { state.gizmo = GizmoMode::Translate; }
                    if keys.just_pressed(KeyCode::KeyE) { state.gizmo = GizmoMode::Rotate; }
                    if keys.just_pressed(KeyCode::KeyR) { state.gizmo = GizmoMode::Scale; }
                    if keys.just_pressed(KeyCode::KeyX) { state.axis_lock = AxisLock::X; }
                    if keys.just_pressed(KeyCode::KeyY) { state.axis_lock = AxisLock::Y; }
                    if keys.just_pressed(KeyCode::KeyZ) { state.axis_lock = AxisLock::Z; }

                    ui.separator();
                    ui.label("Transform");

                    // Translation
                    let mut pos = t.translation.to_array();
                    ui.horizontal(|ui| {
                        ui.label("Position");
                        ui.add(egui::DragValue::new(&mut pos[0]).speed(0.05));
                        ui.add(egui::DragValue::new(&mut pos[1]).speed(0.05));
                        ui.add(egui::DragValue::new(&mut pos[2]).speed(0.05));
                    });
                    t.translation = Vec3::from_array(pos);

                    // Rotation (Euler degrees)
                    let (mut pitch, mut yaw, mut roll) = t.rotation.to_euler(EulerRot::XYZ);
                    let mut deg = [pitch.to_degrees(), yaw.to_degrees(), roll.to_degrees()];
                    ui.horizontal(|ui| {
                        ui.label("Rotation°");
                        ui.add(egui::DragValue::new(&mut deg[0]).speed(0.2));
                        ui.add(egui::DragValue::new(&mut deg[1]).speed(0.2));
                        ui.add(egui::DragValue::new(&mut deg[2]).speed(0.2));
                    });
                    pitch = deg[0].to_radians(); yaw = deg[1].to_radians(); roll = deg[2].to_radians();
                    t.rotation = Quat::from_euler(EulerRot::XYZ, pitch, yaw, roll);

                    // Scale
                    let mut scl = t.scale.to_array();
                    ui.horizontal(|ui| {
                        ui.label("Scale");
                        ui.add(egui::DragValue::new(&mut scl[0]).speed(0.02));
                        ui.add(egui::DragValue::new(&mut scl[1]).speed(0.02));
                        ui.add(egui::DragValue::new(&mut scl[2]).speed(0.02));
                    });
                    t.scale = Vec3::new(scl[0].max(0.001), scl[1].max(0.001), scl[2].max(0.001));
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
    egui::TopBottomPanel::bottom("visual_script_bottom_list")
        .resizable(true)
        .default_height(150.0)
        .show(contexts.ctx_mut(), |ui| {
            ui.heading("Visual Script (List)");
            ui.separator();
            if let Some(e) = selection.entity {
                if let Ok(mut script) = query.get_mut(e) {
                    ui.horizontal(|ui| {
                        ui.checkbox(&mut script.enabled, "Enabled");
                        if ui.button("Add: Rotate").clicked() {
                            script.ops.push(ScriptOp::RotateXYZ { deg_per_sec: [0.0, 45.0, 0.0] });
                        }
                        if ui.button("Add: Translate Y").clicked() {
                            script.ops.push(ScriptOp::TranslateY { units_per_sec: 0.5 });
                        }
                        if ui.button("Add: Scale Pulse").clicked() {
                            script.ops.push(ScriptOp::ScalePulse { base: [1.0,1.0,1.0], amplitude: 0.15, speed_hz: 0.5 });
                        }
                    });
                    ui.separator();

                    let mut remove_idx: Option<usize> = None;
                    for (i, op) in script.ops.iter_mut().enumerate() {
                        ui.group(|ui| {
                            ui.horizontal(|ui| {
                                ui.label(format!("Step {}", i + 1));
                                if ui.small_button("▲").clicked() && i > 0 { script.ops.swap(i, i - 1); }
                                if ui.small_button("▼").clicked() && i + 1 < script.ops.len() { script.ops.swap(i, i + 1); }
                                if ui.small_button("✖").clicked() { remove_idx = Some(i); }
                            });
                            match op {
                                ScriptOp::RotateXYZ { deg_per_sec } => {
                                    ui.label("RotateXYZ (deg/s)");
                                    ui.horizontal(|ui| {
                                        ui.add(egui::DragValue::new(&mut deg_per_sec[0]).speed(1.0));
                                        ui.add(egui::DragValue::new(&mut deg_per_sec[1]).speed(1.0));
                                        ui.add(egui::DragValue::new(&mut deg_per_sec[2]).speed(1.0));
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
                                        ui.add(egui::DragValue::new(&mut base[0]).speed(0.02));
                                        ui.add(egui::DragValue::new(&mut base[1]).speed(0.02));
                                        ui.add(egui::DragValue::new(&mut base[2]).speed(0.02));
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
                    if let Some(idx) = remove_idx { script.ops.remove(idx); }
                } else {
                    ui.label("Selected entity has no VisualScript component.");
                }
            } else {
                ui.label("Nothing selected");
            }
        });
}

/* Node Graph */

fn ui_node_graph(
    mut contexts: EguiContexts,
    selection: Res<Selection>,
    mut q_graph: Query<&mut NodeGraph>,
) {
    egui::Window::new("Node Graph").default_open(true).resizable(true).show(contexts.ctx_mut(), |ui| {
        if let Some(e) = selection.entity {
            if let Ok(mut graph) = q_graph.get_mut(e) {
                ui.horizontal(|ui| {
                    if ui.button("Add Rotate").clicked() {
                        let id = graph.next_id; graph.next_id += 1;
                        graph.nodes.push(Node { id, title: format!("Rotate {}", id), pos: [20.0, 20.0], op: ScriptOp::RotateXYZ { deg_per_sec: [0.0,45.0,0.0] } });
                    }
                    if ui.button("Add ScalePulse").clicked() {
                        let id = graph.next_id; graph.next_id += 1;
                        graph.nodes.push(Node { id, title: format!("ScalePulse {}", id), pos: [220.0, 20.0], op: ScriptOp::ScalePulse { base: [1.0,1.0,1.0], amplitude: 0.15, speed_hz: 0.5 } });
                    }
                });
                ui.separator();

                let painter = ui.painter();
                // Draw links
                for (a, b) in &graph.links {
                    let pa = graph.nodes.iter().find(|n| &n.id == a).map(|n| egui::pos2(n.pos[0]+70.0, n.pos[1]+20.0));
                    let pb = graph.nodes.iter().find(|n| &n.id == b).map(|n| egui::pos2(n.pos[0]+0.0, n.pos[1]+20.0));
                    if let (Some(pa), Some(pb)) = (pa, pb) {
                        painter.add(egui::Shape::bezier_cubic(
                            egui::epaint::CubicBezierShape::from_points_stroke(
                                [pa, egui::pos2((pa.x+pb.x)/2.0, pa.y), egui::pos2((pa.x+pb.x)/2.0, pb.y), pb],
                                false,
                                egui::Color32::TRANSPARENT,
                                egui::Stroke::new(2.0, egui::Color32::from_gray(180)),
                            )
                        ));
                    }
                }

                // Draw nodes (draggable)
                let mut to_remove: Option<u64> = None;
                for node in &mut graph.nodes {
                    let rect = egui::Rect::from_min_size(egui::pos2(node.pos[0], node.pos[1]), egui::vec2(160.0, 48.0));
                    let resp = ui.allocate_rect(rect, egui::Sense::click_and_drag());
                    let fill = egui::Color32::from_rgb(36, 44, 58);
                    painter.rect(rect, 6.0, fill, egui::Stroke::new(1.0, egui::Color32::from_gray(150)));
                    painter.text(rect.left_top() + egui::vec2(8.0, 6.0), egui::Align2::LEFT_TOP, &node.title, egui::FontId::proportional(14.0), egui::Color32::WHITE);

                    if resp.dragged() {
                        node.pos[0] += resp.drag_delta().x;
                        node.pos[1] += resp.drag_delta().y;
                    }

                    if resp.secondary_clicked() {
                        egui::popup::show_context_menu(ui.ctx(), resp.id, |ui| {
                            if ui.button("Delete").clicked() { to_remove = Some(node.id); ui.close_menu(); }
                        });
                    }
                }
                if let Some(id) = to_remove {
                    graph.nodes.retain(|n| n.id != id);
                    graph.links.retain(|(a,b)| *a != id && *b != id);
                }
            } else {
                ui.label("Selected entity has no NodeGraph component.");
            }
        } else {
            ui.label("Select an entity to edit its graph.");
        }
    });
}

/* =====================
   Script Application
   ===================== */

fn apply_scripts(
    time: Res<Time>,
    mut q: Query<(&mut Transform, Option<&VisualScript>, Option<&NodeGraph>)>,
) {
    for (mut t, script, graph) in q.iter_mut() {
        // List-based ops
        if let Some(script) = script {
            if script.enabled {
                for op in &script.ops {
                    apply_op(&mut t, op, time.delta_seconds(), time.elapsed_seconds());
                }
            }
        }
        // Graph-based ops (simple: run all nodes in insertion order)
        if let Some(graph) = graph {
            for node in &graph.nodes {
                apply_op(&mut t, &node.op, time.delta_seconds(), time.elapsed_seconds());
            }
        }
    }
}

fn apply_op(t: &mut Transform, op: &ScriptOp, dt: f32, time_s: f32) {
    use std::f32::consts::PI;
    match op {
        ScriptOp::RotateXYZ { deg_per_sec } => {
            let rad = Vec3::new(deg_per_sec[0], deg_per_sec[1], deg_per_sec[2]).to_radians() * dt;
            let qx = Quat::from_rotation_x(rad.x);
            let qy = Quat::from_rotation_y(rad.y);
            let qz = Quat::from_rotation_z(rad.z);
            t.rotation = qy * qx * qz * t.rotation;
        }
        ScriptOp::TranslateY { units_per_sec } => {
            t.translation.y += units_per_sec * dt;
        }
        ScriptOp::ScalePulse { base, amplitude, speed_hz } => {
            let base = Vec3::from_array(*base);
            let s = base + Vec3::splat(*amplitude) * (2.0 * PI * *speed_hz * time_s).sin();
            t.scale = s.max(Vec3::splat(0.001));
        }
    }
}

/* =====================
   Camera & Gizmo
   ===================== */

fn camera_controls(
    mut windows: Query<&mut Window, With<PrimaryWindow>>,
    mut camera_q: Query<&mut Transform, (With<Camera3d>, Without<DirectionalLight>)>,
    mut orbit: ResMut<CameraOrbit>,
    mouse: Res<ButtonInput<MouseButton>>,
    keys: Res<ButtonInput<KeyCode>>,
    mut motion_evr: EventReader<MouseMotion>,
    mut wheel_evr: EventReader<MouseWheel>,
) {
    let mut cam = if let Ok(c) = camera_q.get_single_mut() { c } else { return; };

    let mut delta = Vec2::ZERO;
    for ev in motion_evr.read() { delta += ev.delta; }

    let mut zoom = 0.0f32;
    for ev in wheel_evr.read() { zoom += ev.y; }
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
        let right = cam.right();
        let up    = cam.up();
        orbit.target += (-right * delta.x + up * delta.y) * orbit.pan_speed * orbit.distance.max(1.0);
    }

    let rot = Quat::from_euler(EulerRot::ZYX, 0.0, orbit.yaw, orbit.pitch);
    cam.translation = orbit.target + rot * (Vec3::Z * orbit.distance);
    cam.look_at(orbit.target, Vec3::Y);

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

fn draw_gizmos(
    selection: Res<Selection>,
    mut gizmos: Gizmos,
    transforms: Query<&Transform>,
) {
    if let Some(e) = selection.entity {
        if let Ok(t) = transforms.get(e) {
            // Draw axis lines at entity origin
            let o = t.translation;
            let len = 1.5;
            gizmos.ray(o, Vec3::X * len, Color::srgb(1.0, 0.2, 0.2));
            gizmos.ray(o, Vec3::Y * len, Color::srgb(0.2, 1.0, 0.2));
            gizmos.ray(o, Vec3::Z * len, Color::srgb(0.2, 0.6, 1.0));
        }
    }
}

fn handle_gizmo_drag(
    mut state: ResMut<EditorState>,
    selection: Res<Selection>,
    mut transforms: Query<&mut Transform>,
    mut windows: Query<&mut Window, With<PrimaryWindow>>,
    buttons: Res<ButtonInput<MouseButton>>,
    mut motion_evr: EventReader<MouseMotion>,
    keys: Res<ButtonInput<KeyCode>>,
    camera_q: Query<(&Camera, &GlobalTransform), With<Camera3d>>,
) {
    let (camera, cam_tf) = if let Ok(c) = camera_q.get_single() { c } else { return; };

    let mut delta = Vec2::ZERO;
    for ev in motion_evr.read() { delta += ev.delta; }

    if keys.just_pressed(KeyCode::KeyW) { state.gizmo = GizmoMode::Translate; }
    if keys.just_pressed(KeyCode::KeyE) { state.gizmo = GizmoMode::Rotate; }
    if keys.just_pressed(KeyCode::KeyR) { state.gizmo = GizmoMode::Scale; }
    if keys.just_pressed(KeyCode::KeyX) { state.axis_lock = AxisLock::X; }
    if keys.just_pressed(KeyCode::KeyY) { state.axis_lock = AxisLock::Y; }
    if keys.just_pressed(KeyCode::KeyZ) { state.axis_lock = AxisLock::Z; }

    if let Some(e) = selection.entity {
        if let Ok(mut t) = transforms.get_mut(e) {
            let dragging = buttons.pressed(MouseButton::Left);
            if dragging && delta.length_squared() > 0.0 {
                match state.gizmo {
                    GizmoMode::Translate => {
                        let mut d = Vec3::ZERO;
                        match state.axis_lock {
                            AxisLock::X => d.x = delta.x * 0.01,
                            AxisLock::Y => d.y = delta.y * 0.01,
                            AxisLock::Z => d.z = -delta.x * 0.01,
                            AxisLock::None => {
                                let right = cam_tf.right();
                                let up = cam_tf.up();
                                t.translation += (-right * delta.x + up * delta.y) * 0.01 * t.translation.distance(cam_tf.translation());
                                return;
                            }
                        }
                        t.translation += d;
                    }
                    GizmoMode::Rotate => {
                        let mut ang = Vec3::ZERO;
                        match state.axis_lock {
                            AxisLock::X => ang.x = delta.y * 0.3_f32.to_radians(),
                            AxisLock::Y => ang.y = -delta.x * 0.3_f32.to_radians(),
                            AxisLock::Z => ang.z = delta.x * 0.3_f32.to_radians(),
                            AxisLock::None => ang.y = -delta.x * 0.3_f32.to_radians(),
                        }
                        t.rotation = Quat::from_euler(EulerRot::XYZ, ang.x, ang.y, ang.z) * t.rotation;
                    }
                    GizmoMode::Scale => {
                        let s = 1.0 + delta.y * -0.005;
                        match state.axis_lock {
                            AxisLock::X => t.scale.x = (t.scale.x * s).max(0.001),
                            AxisLock::Y => t.scale.y = (t.scale.y * s).max(0.001),
                            AxisLock::Z => t.scale.z = (t.scale.z * s).max(0.001),
                            AxisLock::None => t.scale = (t.scale * s).max(Vec3::splat(0.001)),
                        }
                    }
                }
            }
        }
    }

    if let Ok(mut window) = windows.get_single_mut() {
        window.cursor_options.visible = true;
    }
}

/* =====================
   Picking (click in 3D)
   ===================== */

#[derive(Component)]
struct PickBounds {
    radius: f32, // bounding sphere radius in world units (approx)
}

fn screen_to_world_ray(
    camera: &Camera,
    cam_tf: &GlobalTransform,
    cursor_pos: Vec2,
    window: &Window,
) -> Option<(Vec3, Vec3)> {
    let ray = camera.viewport_to_world(cam_tf, cursor_pos).ok()?;
    Some((ray.origin, *ray.direction))
}

fn picking_by_click(
    mut selection: ResMut<Selection>,
    buttons: Res<ButtonInput<MouseButton>>,
    windows_q: Query<&Window, With<PrimaryWindow>>,
    camera_q: Query<(&Camera, &GlobalTransform), With<Camera3d>>,
    transforms: Query<(Entity, &Transform, Option<&Name>, Option<&PickBounds>)>,
    mut egui_ctxs: EguiContexts,
) {
    // Ignore if UI is interacting
    let ctx = egui_ctxs.ctx_mut();
    if ctx.wants_pointer_input() { return; }

    if !buttons.just_pressed(MouseButton::Left) { return; }
    let window = if let Ok(w) = windows_q.get_single() { w } else { return; };
    let (camera, cam_tf) = if let Ok(c) = camera_q.get_single() { c } else { return; };

    if let Some(cursor) = window.cursor_position() {
        if let Some((ro, rd)) = screen_to_world_ray(camera, cam_tf, cursor, window) {
            // simple sphere picking: choose closest hit
            let mut best: Option<(Entity, f32)> = None;
            for (e, t, _name, b) in transforms.iter() {
                let r = b.map(|b| b.radius).unwrap_or(1.0) * t.scale.max_element();
                // Ray-sphere intersection at center t.translation
                let oc = ro - t.translation;
                let a = rd.length_squared();
                let bq = 2.0 * oc.dot(rd);
                let c = oc.length_squared() - r * r;
                let disc = bq*bq - 4.0*a*c;
                if disc >= 0.0 {
                    let t0 = (-bq - disc.sqrt()) / (2.0*a);
                    if t0 > 0.0 {
                        if let Some((_, best_t)) = best {
                            if t0 < best_t { best = Some((e, t0)); }
                        } else {
                            best = Some((e, t0));
                        }
                    }
                }
            }
            if let Some((hit, _)) = best {
                selection.entity = Some(hit);
            }
        }
    }
}

/* =====================
   Helpers
   ===================== */

trait Vec3Ext {
    fn max_element(&self) -> f32;
}
impl Vec3Ext for Vec3 {
    fn max_element(&self) -> f32 { self.x.max(self.y).max(self.z) }
}
