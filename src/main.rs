//! Sprout Engine — Unified with FBX + Enhanced Editor Features
//! A free, open-source alternative to Game Engine built on Bevy
//! Features: advanced editor UI, material system, project management, visual scripting, and more

use bevy::prelude::*;
use bevy::window::{PrimaryWindow, WindowResolution};
use bevy_egui::{egui, EguiContexts, EguiPlugin};
use serde::{Deserialize, Serialize};
use std::fs;

use bevy_mod_fbx::FbxPlugin;

// Module declarations
mod viewport;
mod gizmo;
mod material;
mod project;

use viewport::ViewportPlugin;
use gizmo::GizmoPlugin;
use material::MaterialPlugin;
use project::ProjectPlugin;

fn main() {
    App::new()
        .insert_resource(ClearColor(Color::hex("14161a").unwrap()))
        .insert_resource(EditorState::default())
        .insert_resource(Selection::default())
        .insert_resource(CameraOrbit::default())
        .add_plugins(
            DefaultPlugins
                .set(WindowPlugin {
                    primary_window: Some(Window {
                        title: "Sprout Engine — Advanced Editor".into(),
                        resolution: WindowResolution::new(1600.0, 1000.0),
                        resizable: true,
                        ..default()
                    }),
                    ..default()
                })
                .set(ImagePlugin::default_nearest()),
        )
        .add_plugin(EguiPlugin)
        .add_plugin(FbxPlugin) // FBX support
        .add_plugin(ViewportPlugin) // Enhanced viewport system
        .add_plugin(GizmoPlugin) // Advanced gizmo system
        .add_plugin(MaterialPlugin) // Material editor
        .add_plugin(ProjectPlugin) // Project management
        .add_startup_system(setup_scene)
        .add_systems(
            (
                ui_menu,
                ui_hierarchy,
                ui_inspector,
                ui_script_panel,
                ui_node_graph,
                ui_content_browser,
                ui_animator_tab,
                apply_scripts,
                camera_controls,
                frame_selected_on_f_key,
                draw_gizmos,
                handle_gizmo_drag,
                picking_select,
            ),
        )
        .run();
}

/* =====================
   Data & Components
   ===================== */

#[derive(Resource, Default)]
struct Selection { entity: Option<Entity> }

#[derive(Clone, Serialize, Deserialize)]
enum ScriptOp {
    RotateXYZ { deg_per_sec: [f32; 3] },
    TranslateY { units_per_sec: f32 },
    ScalePulse { base: [f32; 3], amplitude: f32, speed_hz: f32 },
}
impl Default for ScriptOp {
    fn default() -> Self { ScriptOp::RotateXYZ { deg_per_sec: [0.0, 45.0, 0.0] } }
}

#[derive(Component, Default, Serialize, Deserialize, Clone)]
struct VisualScript { enabled: bool, ops: Vec<ScriptOp> }

#[derive(Clone, Serialize, Deserialize)]
struct Node { id: u64, title: String, pos: [f32; 2], op: ScriptOp }

#[derive(Component, Default, Serialize, Deserialize, Clone)]
struct NodeGraph { nodes: Vec<Node>, links: Vec<(u64,u64)>, next_id: u64 }

/* Animator: minimal state machine with one float parameter `speed` */

#[derive(Component, Default, Serialize, Deserialize, Clone)]
struct Animator {
    /// Name of the selected clip to play (for simple mode)
    current_clip: Option<String>,
    /// Global playback speed
    speed: f32,
    /// Whether to loop
    looped: bool,
    /// Optional simple "speed threshold" state machine
    states: Vec<AnimState>,
    parameter_speed: f32, // exposed in UI
    active_state: Option<String>,
}

#[derive(Clone, Serialize, Deserialize)]
struct AnimState {
    name: String,
    clip: String,
    start_threshold: f32, // enter when parameter_speed >= start_threshold
}

#[derive(Component)]
struct AnimationBindings {
    clips: Vec<String>, // discovered clip names
}

#[derive(Serialize, Deserialize)]
struct SceneEntity {
    name: String,
    translation: [f32; 3],
    rotation_euler_deg: [f32; 3],
    scale: [f32; 3],
    script: Option<VisualScript>,
    graph: Option<NodeGraph>,
    animator: Option<Animator>,
}

#[derive(Serialize, Deserialize, Default)]
struct SceneFile { entities: Vec<SceneEntity> }

#[derive(Resource)]
struct EditorState {
    file_path: String,
    gizmo: GizmoMode,
    axis_lock: AxisLock,
    assets_scan: Vec<String>,
    assets_root: String,
}
impl Default for EditorState {
    fn default() -> Self {
        Self {
            file_path: "sprout_scene.json".into(),
            gizmo: GizmoMode::Translate,
            axis_lock: AxisLock::None,
            assets_scan: vec![],
            assets_root: "assets".into(),
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
struct CameraOrbit { target: Vec3, distance: f32, yaw: f32, pitch: f32, pan_speed: f32, orbit_speed: f32, zoom_speed: f32 }
impl Default for CameraOrbit {
    fn default() -> Self {
        Self { target: Vec3::ZERO, distance: 6.0, yaw: 0.5, pitch: 0.25, pan_speed: 0.005, orbit_speed: 0.005, zoom_speed: 1.0 }
    }
}

/* =====================
   Startup
   ===================== */

fn setup_scene(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    commands.spawn((
        Name::new("Ground"),
        PbrBundle {
            mesh: meshes.add(Mesh::from(shape::Plane { size: 20.0, subdivisions: 0 })),
            material: materials.add(StandardMaterial {
                base_color: Color::rgb(0.08, 0.1, 0.12),
                perceptual_roughness: 1.0,
                ..default()
            }),
            ..default()
        },
    ));

    commands.spawn((
        Name::new("Cube"),
        PbrBundle {
            mesh: meshes.add(Mesh::from(shape::Cube { size: 1.0 })),
            material: materials.add(StandardMaterial { base_color: Color::rgb(0.36, 0.72, 1.0), ..default() }),
            transform: Transform::from_xyz(0.0, 0.5, 0.0),
            ..default()
        },
        VisualScript { enabled: true, ops: vec![ScriptOp::RotateXYZ { deg_per_sec: [0.0,45.0,0.0] }] },
        NodeGraph { nodes: vec![], links: vec![], next_id: 1 },
        Animator { speed: 1.0, looped: true, ..Default::default() },
        AnimationBindings { clips: vec![] },
    ));

    // Light
    commands.spawn((
        Name::new("Sun"),
        DirectionalLightBundle {
            directional_light: DirectionalLight { shadows_enabled: true, illuminance: 8000.0, ..default() },
            transform: Transform::from_xyz(5.0, 10.0, 6.0).looking_at(Vec3::ZERO, Vec3::Y),
            ..default()
        },
    ));

    // Camera
    commands.spawn((
        Name::new("EditorCamera"),
        Camera3dBundle {
            transform: Transform::from_xyz(0.0, 3.0, 8.0).looking_at(Vec3::ZERO, Vec3::Y),
            ..default()
        },
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
    q_full: Query<(Entity, &Name, &Transform, Option<&VisualScript>, Option<&NodeGraph>, Option<&Animator>)>,
) {
    egui::TopBottomPanel::top("menu_top").show(contexts.ctx_mut(), |ui| {
        egui::menu::bar(ui, |ui| {
            ui.menu_button("File", |ui| {
                if ui.button("New Scene").clicked() {
                    for (e, name) in q_entities.iter() {
                        if name.as_str() != "EditorCamera" && name.as_str() != "Sun" {
                            commands.entity(e).despawn_recursive();
                        }
                    }
                    commands.spawn((
                        Name::new("Cube"),
                        PbrBundle {
                            mesh: meshes.add(Mesh::from(shape::Cube { size: 1.0 })),
                            material: materials.add(StandardMaterial { base_color: Color::rgb(0.36, 0.72, 1.0), ..default() }),
                            transform: Transform::from_xyz(0.0, 0.5, 0.0),
                            ..default()
                        },
                        VisualScript::default(),
                        NodeGraph::default(),
                        Animator { speed: 1.0, looped: true, ..Default::default() },
                        AnimationBindings { clips: vec![] },
                    ));
                    ui.close_menu();
                }
                ui.separator();
                ui.horizontal(|ui|{
                    ui.label("Path:");
                    ui.text_edit_singleline(&mut state.file_path);
                });
                if ui.button("Save").clicked() {
                    let mut file = SceneFile { entities: vec![] };
                    for (_, name, t, script, graph, anim) in q_full.iter() {
                        if name.as_str() == "EditorCamera" { continue; }
                        let (pitch, yaw, roll) = t.rotation.to_euler(EulerRot::XYZ);
                        file.entities.push(SceneEntity {
                            name: name.to_string(),
                            translation: t.translation.to_array(),
                            rotation_euler_deg: [pitch.to_degrees(), yaw.to_degrees(), roll.to_degrees()],
                            scale: t.scale.to_array(),
                            script: script.cloned(),
                            graph: graph.cloned(),
                            animator: anim.cloned(),
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
                            for (e, name) in q_entities.iter() {
                                if name.as_str() != "EditorCamera" && name.as_str() != "Sun" {
                                    commands.entity(e).despawn_recursive();
                                }
                            }
                            for ent in scene.entities {
                                let entity = commands.spawn((
                                    Name::new(ent.name),
                                    PbrBundle {
                                        mesh: meshes.add(Mesh::from(shape::Cube { size: 1.0 })),
                                        material: materials.add(StandardMaterial { base_color: Color::rgb(0.36, 0.72, 1.0), ..default() }),
                                        transform: Transform {
                                            translation: Vec3::from_array(ent.translation),
                                            rotation: Quat::from_euler(EulerRot::XYZ, ent.rotation_euler_deg[0].to_radians(), ent.rotation_euler_deg[1].to_radians(), ent.rotation_euler_deg[2].to_radians()),
                                            scale: Vec3::from_array(ent.scale),
                                        },
                                        ..default()
                                    },
                                    AnimationBindings { clips: vec![] },
                                )).id();
                                if let Some(s) = ent.script { commands.entity(entity).insert(s); }
                                if let Some(g) = ent.graph { commands.entity(entity).insert(g); }
                                if let Some(a) = ent.animator { commands.entity(entity).insert(a); }
                            }
                        }
                    }
                    ui.close_menu();
                }
            });

            ui.menu_button("Gizmo", |ui| {
                ui.horizontal(|ui| {
                    ui.selectable_value(&mut state.gizmo, GizmoMode::Translate, "W Move");
                    ui.selectable_value(&mut state.gizmo, GizmoMode::Rotate,   "E Rotate");
                    ui.selectable_value(&mut state.gizmo, GizmoMode::Scale,    "R Scale");
                });
                ui.separator();
                ui.horizontal(|ui| {
                    ui.selectable_value(&mut state.axis_lock, AxisLock::None, "Free");
                    ui.selectable_value(&mut state.axis_lock, AxisLock::X, "X");
                    ui.selectable_value(&mut state.axis_lock, AxisLock::Y, "Y");
                    ui.selectable_value(&mut state.axis_lock, AxisLock::Z, "Z");
                });
            });
        });
    });
}

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
                    if ui.selectable_label(selected, label).clicked() {
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
    keys: Res<Input<KeyCode>>,
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
                    if keys.just_pressed(KeyCode::W) { state.gizmo = GizmoMode::Translate; }
                    if keys.just_pressed(KeyCode::E) { state.gizmo = GizmoMode::Rotate; }
                    if keys.just_pressed(KeyCode::R) { state.gizmo = GizmoMode::Scale; }
                    if keys.just_pressed(KeyCode::X) { state.axis_lock = AxisLock::X; }
                    if keys.just_pressed(KeyCode::Y) { state.axis_lock = AxisLock::Y; }
                    if keys.just_pressed(KeyCode::Z) { state.axis_lock = AxisLock::Z; }

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
    egui::TopBottomPanel::bottom("visual_script_bottom")
        .resizable(true)
        .default_height(140.0)
        .show(contexts.ctx_mut(), |ui| {
            ui.heading("Visual Script (List)");
            ui.separator();
            if let Some(e) = selection.entity {
                if let Ok(mut script) = query.get_mut(e) {
                    ui.horizontal(|ui| {
                        ui.checkbox(&mut script.enabled, "Enabled");
                        if ui.button("Add: Rotate").clicked() { script.ops.push(ScriptOp::RotateXYZ { deg_per_sec: [0.0,45.0,0.0] }); }
                        if ui.button("Add: Translate Y").clicked() { script.ops.push(ScriptOp::TranslateY { units_per_sec: 0.5 }); }
                        if ui.button("Add: Scale Pulse").clicked() { script.ops.push(ScriptOp::ScalePulse { base: [1.0,1.0,1.0], amplitude: 0.15, speed_hz: 0.5 }); }
                    });
                    ui.separator();
                    let mut remove_idx: Option<usize> = None;
                    let mut move_up: Option<usize> = None;
                    let mut move_down: Option<usize> = None;
                    let ops_len = script.ops.len();

                    for (i, op) in script.ops.iter_mut().enumerate() {
                        ui.group(|ui| {
                            ui.horizontal(|ui| {
                                ui.label(format!("Step {}", i+1));
                                if ui.small_button("▲").clicked() && i>0 { move_up = Some(i); }
                                if ui.small_button("▼").clicked() && i+1<ops_len { move_down = Some(i); }
                                if ui.small_button("✖").clicked() { remove_idx = Some(i); }
                            });
                            match op {
                                ScriptOp::RotateXYZ { deg_per_sec } => {
                                    ui.label("RotateXYZ (deg/s)");
                                    ui.horizontal(|ui|{
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
                                        ui.label("Base"); ui.add(egui::DragValue::new(&mut base[0]).speed(0.02));
                                        ui.label("");     ui.add(egui::DragValue::new(&mut base[1]).speed(0.02));
                                        ui.label("");     ui.add(egui::DragValue::new(&mut base[2]).speed(0.02));
                                    });
                                    ui.horizontal(|ui| {
                                        ui.label("Amplitude"); ui.add(egui::DragValue::new(amplitude).speed(0.01));
                                        ui.label("Speed (Hz)"); ui.add(egui::DragValue::new(speed_hz).speed(0.01));
                                    });
                                }
                            }
                        });
                    }

                    if let Some(idx) = remove_idx { script.ops.remove(idx); }
                    if let Some(idx) = move_up { script.ops.swap(idx, idx-1); }
                    if let Some(idx) = move_down { script.ops.swap(idx, idx+1); }
                } else { ui.label("Selected entity has no VisualScript."); }
            } else { ui.label("Nothing selected."); }
        });
}

/* Node Graph – minimal */
fn ui_node_graph(mut contexts: EguiContexts, selection: Res<Selection>, mut q_graph: Query<&mut NodeGraph>) {
    egui::Window::new("Node Graph").default_open(false).show(contexts.ctx_mut(), |ui| {
        if let Some(e) = selection.entity {
            if let Ok(mut graph) = q_graph.get_mut(e) {
                ui.horizontal(|ui| {
                    if ui.button("Add Rotate").clicked() {
                        let id = graph.next_id; graph.next_id += 1;
                        graph.nodes.push(Node { id, title: format!("Rotate {id}"), pos: [20.0,20.0], op: ScriptOp::RotateXYZ{deg_per_sec:[0.0,45.0,0.0]} });
                    }
                    if ui.button("Add ScalePulse").clicked() {
                        let id = graph.next_id; graph.next_id += 1;
                        graph.nodes.push(Node { id, title: format!("ScalePulse {id}"), pos: [220.0,20.0], op: ScriptOp::ScalePulse{base:[1.0,1.0,1.0], amplitude:0.15, speed_hz:0.5} });
                    }
                });
                ui.separator();
                // Just list nodes for now (full canvas omitted for brevity)
                for n in &graph.nodes {
                    ui.label(format!("{} at ({:.0},{:.0})", n.title, n.pos[0], n.pos[1]));
                }
            } else { ui.label("Selected entity has no NodeGraph."); }
        } else { ui.label("Select an entity."); }
    });
}

/* Content Browser */
fn ui_content_browser(
    mut contexts: EguiContexts,
    mut state: ResMut<EditorState>,
    mut commands: Commands,
    asset_server: Res<AssetServer>,
) {
    egui::TopBottomPanel::bottom("content_browser").default_height(150.0).resizable(true).show(contexts.ctx_mut(), |ui| {
        ui.horizontal(|ui| {
            ui.heading("Content Browser");
            if ui.button("Refresh").clicked() {
                state.assets_scan = scan_assets(&state.assets_root);
            }
        });
        ui.separator();
        if state.assets_scan.is_empty() { state.assets_scan = scan_assets(&state.assets_root); }
        egui::ScrollArea::horizontal().show(ui, |ui| {
            for path in &state.assets_scan {
                ui.group(|ui| {
                    ui.label(path);
                    if ui.button("Spawn").clicked() {
                        spawn_asset(&mut commands, &asset_server, path);
                    }
                });
                ui.add_space(8.0);
            }
        });
    });
}

fn scan_assets(root: &str) -> Vec<String> {
    let exts = ["gltf","glb","fbx","png","jpg","jpeg"];
    let mut out = vec![];
    if let Ok(read) = std::fs::read_dir(root) {
        for e in read.flatten() {
            if let Some(ext) = e.path().extension().and_then(|s| s.to_str()).map(|s| s.to_lowercase()) {
                if exts.contains(&ext.as_str()) {
                    if let Some(rel) = e.path().to_str() {
                        out.push(rel.replace('\\',"/").to_string());
                    }
                }
            }
        }
    }
    out
}

fn spawn_asset(commands: &mut Commands, asset_server: &AssetServer, path: &str) {
    if path.ends_with(".png") || path.ends_with(".jpg") || path.ends_with(".jpeg") {
        // billboard quad
        commands.spawn((
            Name::new(format!("Image {}", path)),
            SpriteBundle {
                texture: asset_server.load(path),
                transform: Transform::from_xyz(0.0, 0.5, 0.0),
                ..default()
            },
        ));
    } else if path.ends_with(".gltf") || path.ends_with(".glb") || path.ends_with(".fbx") {
        // load as scene
        let scene = asset_server.load(path);
        commands.spawn((
            Name::new(format!("Model {}", path)),
            SceneBundle { scene, transform: Transform::from_xyz(0.0, 0.0, 0.0), ..default() },
            Animator::default(),
            AnimationBindings { clips: vec![] },
        ));
    }
}

/* Animator Tab */
fn ui_animator_tab(
    mut contexts: EguiContexts,
    selection: Res<Selection>,
    mut q_anim: Query<(&mut Animator, &AnimationBindings)>,
    _asset_server: Res<AssetServer>,
) {
    egui::Window::new("Animator").default_open(true).resizable(true).show(contexts.ctx_mut(), |ui| {
        if let Some(e) = selection.entity {
            if let Ok((mut anim, bindings)) = q_anim.get_mut(e) {
                ui.horizontal(|ui| {
                    ui.label("Parameter: speed");
                    ui.add(egui::DragValue::new(&mut anim.parameter_speed).speed(0.1));
                });
                ui.horizontal(|ui| {
                    ui.checkbox(&mut anim.looped, "Loop");
                    ui.add(egui::DragValue::new(&mut anim.speed).prefix("Speed x").speed(0.05));
                });
                ui.separator();
                ui.heading("Clips");
                if bindings.clips.is_empty() {
                    // Attempt to list clips from asset metadata (works for glTF that embed animation names).
                    ui.label("Clips list populates when the model with animations finishes loading (glTF recommended).");
                } else {
                    for name in &bindings.clips {
                        let is_sel = anim.current_clip.as_ref().map(|s| s==name).unwrap_or(false);
                        if ui.selectable_label(is_sel, name).clicked() {
                            anim.current_clip = Some(name.clone());
                            anim.active_state = None; // override state machine
                        }
                    }
                }
                ui.separator();
                ui.heading("Simple State Machine");
                if ui.button("Add State").clicked() {
                    let len = anim.states.len();
                    let current_clip = anim.current_clip.clone().unwrap_or_default();
                    anim.states.push(AnimState { name: format!("State{}", len+1), clip: current_clip, start_threshold: 0.0 });
                }
                let mut remove: Option<usize> = None;
                for (i, st) in anim.states.iter_mut().enumerate() {
                    ui.group(|ui| {
                        ui.horizontal(|ui| {
                            ui.text_edit_singleline(&mut st.name);
                            if ui.button("✖").clicked() { remove = Some(i); }
                        });
                        ui.horizontal(|ui| {
                            ui.label("Clip");
                            let mut clip = st.clip.clone();
                            ui.text_edit_singleline(&mut clip);
                            st.clip = clip;
                        });
                        ui.horizontal(|ui| {
                            ui.label("Start threshold");
                            ui.add(egui::DragValue::new(&mut st.start_threshold).speed(0.1));
                        });
                    });
                }
                if let Some(i)=remove { anim.states.remove(i); }

                ui.separator();
                ui.label("Tip: For reliable animations use GLB/GLTF. FBX support depends on plugin capabilities.");
            } else {
                ui.label("Selected entity has no Animator. Spawn a model to attach one.");
            }
        } else {
            ui.label("Select a model entity to edit animations.");
        }
    });
}

/* Apply Animations: very basic playback based on parameter/state machine */
fn apply_scripts(
    time: Res<Time>,
    mut q: Query<(&mut Transform, Option<&VisualScript>, Option<&NodeGraph>)>,
) {
    for (mut t, script, graph) in q.iter_mut() {
        if let Some(script) = script {
            if script.enabled {
                for op in &script.ops {
                    apply_op(&mut t, op, time.delta_seconds(), time.elapsed_seconds());
                }
            }
        }
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
            let rad = Vec3::new(deg_per_sec[0], deg_per_sec[1], deg_per_sec[2]) * dt * std::f32::consts::PI / 180.0;
            let qx = Quat::from_rotation_x(rad.x);
            let qy = Quat::from_rotation_y(rad.y);
            let qz = Quat::from_rotation_z(rad.z);
            t.rotation = qy * qx * qz * t.rotation;
        }
        ScriptOp::TranslateY { units_per_sec } => { t.translation.y += units_per_sec * dt; }
        ScriptOp::ScalePulse { base, amplitude, speed_hz } => {
            let base = Vec3::from_array(*base);
            let s = base + Vec3::splat(*amplitude) * (2.0 * PI * *speed_hz * time_s).sin();
            t.scale = s.max(Vec3::splat(0.001));
        }
    }
}

/* Camera & Gizmo and Picking */
fn camera_controls(
    mut windows: Query<&mut Window, With<PrimaryWindow>>,
    mut camera_q: Query<&mut Transform, (With<Camera3d>, Without<DirectionalLight>)>,
    mut orbit: ResMut<CameraOrbit>,
    mouse: Res<Input<MouseButton>>,
    keys: Res<Input<KeyCode>>,
    mut motion_evr: EventReader<bevy::input::mouse::MouseMotion>,
    mut wheel_evr: EventReader<bevy::input::mouse::MouseWheel>,
) {
    let mut cam = if let Ok(c) = camera_q.get_single_mut() { c } else { return; };

    let mut delta = Vec2::ZERO;
    for ev in motion_evr.iter() { delta += ev.delta; }

    let mut zoom = 0.0f32;
    for ev in wheel_evr.iter() { zoom += ev.y; }
    if zoom.abs() > 0.0 { orbit.distance = (orbit.distance * (1.0 - zoom * 0.1 * orbit.zoom_speed)).clamp(0.5, 100.0); }

    if mouse.pressed(MouseButton::Right) && delta.length_squared() > 0.0 {
        orbit.yaw   -= delta.x * orbit.orbit_speed;
        orbit.pitch -= delta.y * orbit.orbit_speed;
        orbit.pitch = orbit.pitch.clamp(-1.54, 1.54);
    }
    if mouse.pressed(MouseButton::Middle) || (mouse.pressed(MouseButton::Right) && keys.any_pressed([KeyCode::LShift, KeyCode::RShift])) {
        let right = cam.right(); let up = cam.up();
        let pan_speed = orbit.pan_speed;
        let distance = orbit.distance;
        orbit.target += (-right * delta.x + up * delta.y) * pan_speed * distance.max(1.0);
    }
    let rot = Quat::from_euler(EulerRot::ZYX, 0.0, orbit.yaw, orbit.pitch);
    cam.translation = orbit.target + rot * (Vec3::Z * orbit.distance);
    cam.look_at(orbit.target, Vec3::Y);

    if let Ok(mut window) = windows.get_single_mut() { window.cursor.visible = !mouse.pressed(MouseButton::Right); }
}

fn frame_selected_on_f_key(
    selection: Res<Selection>, keys: Res<Input<KeyCode>>,
    mut orbit: ResMut<CameraOrbit>, transforms: Query<&Transform>,
) {
    if keys.just_pressed(KeyCode::F) {
        if let Some(e) = selection.entity { if let Ok(t) = transforms.get(e) { orbit.target = t.translation; } }
    }
}

fn draw_gizmos(_selection: Res<Selection>, _transforms: Query<&Transform>) {
    // Note: Bevy 0.10 gizmos work differently than newer versions
    // For now, we'll disable gizmo rendering to get the project running
}

fn handle_gizmo_drag(
    mut state: ResMut<EditorState>,
    selection: Res<Selection>,
    mut transforms: Query<&mut Transform>,
    buttons: Res<Input<MouseButton>>,
    keys: Res<Input<KeyCode>>,
    mut motion_evr: EventReader<bevy::input::mouse::MouseMotion>,
    camera_q: Query<&GlobalTransform, With<Camera3d>>,
) {
    let cam_tf = if let Ok(c) = camera_q.get_single() { c } else { return; };

    let mut delta = Vec2::ZERO;
    for ev in motion_evr.iter() { delta += ev.delta; }

    if keys.just_pressed(KeyCode::W) { state.gizmo = GizmoMode::Translate; }
    if keys.just_pressed(KeyCode::E) { state.gizmo = GizmoMode::Rotate; }
    if keys.just_pressed(KeyCode::R) { state.gizmo = GizmoMode::Scale; }
    if keys.just_pressed(KeyCode::X) { state.axis_lock = AxisLock::X; }
    if keys.just_pressed(KeyCode::Y) { state.axis_lock = AxisLock::Y; }
    if keys.just_pressed(KeyCode::Z) { state.axis_lock = AxisLock::Z; }

    if let Some(e) = selection.entity {
        if let Ok(mut t) = transforms.get_mut(e) {
            if buttons.pressed(MouseButton::Left) && delta.length_squared() > 0.0 {
                match state.gizmo {
                    GizmoMode::Translate => {
                        let mut d = Vec3::ZERO;
                        match state.axis_lock {
                            AxisLock::X => d.x = delta.x * 0.01,
                            AxisLock::Y => d.y = delta.y * 0.01,
                            AxisLock::Z => d.z = -delta.x * 0.01,
                            AxisLock::None => {
                                let right = cam_tf.right(); let up = cam_tf.up();
                                let translation = t.translation;
                                let distance = translation.distance(cam_tf.translation());
                                t.translation += (-right * delta.x + up * delta.y) * 0.01 * distance;
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
}

/* Picking (very simple AABB raycast against Transforms for primitives; for scenes we don't build meshes here, this is minimal) */
fn picking_select(
    _buttons: Res<Input<MouseButton>>,
) {
    // For brevity, selection is by UI list or prior systems; viewport ray-pick is non-trivial without rendering data.
    // This stub keeps click-to-select logic minimal (handled elsewhere).
    // if buttons.just_pressed(MouseButton::Left) {
        // no-op; in a full editor you'd raycast scene and set selection.entity accordingly
    // }
}
