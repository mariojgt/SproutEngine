// Enhanced gizmo system for Sprout Engine
// Provides visual manipulation tools for objects in the scene

use bevy::prelude::*;
use bevy_egui::{egui, EguiContexts};

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum GizmoTool {
    Select,
    Translate,
    Rotate,
    Scale,
    Universal, // Combined translate/rotate/scale
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum GizmoSpace {
    World,
    Local,
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum SnapMode {
    None,
    Grid,
    Increment,
}

#[derive(Resource)]
pub struct GizmoSettings {
    pub current_tool: GizmoTool,
    pub coordinate_space: GizmoSpace,
    pub snap_mode: SnapMode,
    pub snap_value: f32,
    pub gizmo_size: f32,
    pub show_gizmo: bool,
}

impl Default for GizmoSettings {
    fn default() -> Self {
        Self {
            current_tool: GizmoTool::Translate,
            coordinate_space: GizmoSpace::World,
            snap_mode: SnapMode::None,
            snap_value: 1.0,
            gizmo_size: 1.0,
            show_gizmo: true,
        }
    }
}

#[derive(Component)]
pub struct GizmoHandle {
    pub axis: GizmoAxis,
    pub tool: GizmoTool,
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum GizmoAxis {
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ,
    Center,
}

pub fn gizmo_toolbar(
    mut contexts: EguiContexts,
    mut gizmo_settings: ResMut<GizmoSettings>,
    keys: Res<Input<KeyCode>>,
) {
    // Keyboard shortcuts
    if keys.just_pressed(KeyCode::Q) {
        gizmo_settings.current_tool = GizmoTool::Select;
    }
    if keys.just_pressed(KeyCode::W) {
        gizmo_settings.current_tool = GizmoTool::Translate;
    }
    if keys.just_pressed(KeyCode::E) {
        gizmo_settings.current_tool = GizmoTool::Rotate;
    }
    if keys.just_pressed(KeyCode::R) {
        gizmo_settings.current_tool = GizmoTool::Scale;
    }
    if keys.just_pressed(KeyCode::T) {
        gizmo_settings.current_tool = GizmoTool::Universal;
    }

    egui::TopBottomPanel::top("gizmo_toolbar").show(contexts.ctx_mut(), |ui| {
        ui.horizontal(|ui| {
            ui.label("Tools:");
            
            if ui.selectable_label(gizmo_settings.current_tool == GizmoTool::Select, "Select (Q)").clicked() {
                gizmo_settings.current_tool = GizmoTool::Select;
            }
            
            if ui.selectable_label(gizmo_settings.current_tool == GizmoTool::Translate, "Move (W)").clicked() {
                gizmo_settings.current_tool = GizmoTool::Translate;
            }
            
            if ui.selectable_label(gizmo_settings.current_tool == GizmoTool::Rotate, "Rotate (E)").clicked() {
                gizmo_settings.current_tool = GizmoTool::Rotate;
            }
            
            if ui.selectable_label(gizmo_settings.current_tool == GizmoTool::Scale, "Scale (R)").clicked() {
                gizmo_settings.current_tool = GizmoTool::Scale;
            }
            
            if ui.selectable_label(gizmo_settings.current_tool == GizmoTool::Universal, "Universal (T)").clicked() {
                gizmo_settings.current_tool = GizmoTool::Universal;
            }

            ui.separator();

            ui.label("Space:");
            ui.selectable_value(&mut gizmo_settings.coordinate_space, GizmoSpace::World, "World");
            ui.selectable_value(&mut gizmo_settings.coordinate_space, GizmoSpace::Local, "Local");

            ui.separator();

            ui.label("Snap:");
            ui.selectable_value(&mut gizmo_settings.snap_mode, SnapMode::None, "None");
            ui.selectable_value(&mut gizmo_settings.snap_mode, SnapMode::Grid, "Grid");
            ui.selectable_value(&mut gizmo_settings.snap_mode, SnapMode::Increment, "Increment");
            
            if gizmo_settings.snap_mode != SnapMode::None {
                ui.add(egui::DragValue::new(&mut gizmo_settings.snap_value).speed(0.1).clamp_range(0.1..=10.0));
            }

            ui.separator();

            ui.label("Size:");
            ui.add(egui::DragValue::new(&mut gizmo_settings.gizmo_size).speed(0.1).clamp_range(0.1..=5.0));
        });
    });
}

pub fn spawn_gizmo_handles(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    // This would spawn invisible collision meshes for gizmo interaction
    // For now, we'll create a simple representation
    
    let handle_material = materials.add(StandardMaterial {
        base_color: Color::rgba(1.0, 0.0, 0.0, 0.8),
        unlit: true,
        alpha_mode: AlphaMode::Blend,
        ..default()
    });

    // X-axis handle (red)
    commands.spawn((
        Name::new("GizmoHandle_X"),
        PbrBundle {
            mesh: meshes.add(Mesh::from(shape::Capsule {
                radius: 0.05,
                depth: 2.0,
                ..default()
            })),
            material: handle_material.clone(),
            transform: Transform::from_rotation(Quat::from_rotation_z(std::f32::consts::PI / 2.0)),
            visibility: Visibility::Hidden,
            ..default()
        },
        GizmoHandle {
            axis: GizmoAxis::X,
            tool: GizmoTool::Translate,
        },
    ));
}

pub fn update_gizmo_visibility(
    gizmo_settings: Res<GizmoSettings>,
    selection: Res<crate::Selection>,
    mut gizmo_handles: Query<&mut Visibility, With<GizmoHandle>>,
    _transforms: Query<&Transform, Without<GizmoHandle>>,
) {
    let show_gizmos = gizmo_settings.show_gizmo && 
                     gizmo_settings.current_tool != GizmoTool::Select &&
                     selection.entity.is_some();

    for mut visibility in gizmo_handles.iter_mut() {
        *visibility = if show_gizmos {
            Visibility::Visible
        } else {
            Visibility::Hidden
        };
    }
}

pub fn position_gizmos(
    selection: Res<crate::Selection>,
    gizmo_settings: Res<GizmoSettings>,
    selected_transforms: Query<&Transform, Without<GizmoHandle>>,
    mut gizmo_transforms: Query<&mut Transform, With<GizmoHandle>>,
) {
    if let Some(selected_entity) = selection.entity {
        if let Ok(selected_transform) = selected_transforms.get(selected_entity) {
            for mut gizmo_transform in gizmo_transforms.iter_mut() {
                gizmo_transform.translation = selected_transform.translation;
                
                // Apply coordinate space
                match gizmo_settings.coordinate_space {
                    GizmoSpace::World => {
                        // Keep world-aligned rotation
                    }
                    GizmoSpace::Local => {
                        gizmo_transform.rotation = selected_transform.rotation;
                    }
                }
                
                // Apply gizmo size
                gizmo_transform.scale = Vec3::splat(gizmo_settings.gizmo_size);
            }
        }
    }
}

pub fn apply_snap(value: f32, snap_mode: SnapMode, snap_value: f32) -> f32 {
    match snap_mode {
        SnapMode::None => value,
        SnapMode::Grid | SnapMode::Increment => {
            (value / snap_value).round() * snap_value
        }
    }
}

pub struct GizmoPlugin;

impl Plugin for GizmoPlugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<GizmoSettings>()
            .add_startup_system(spawn_gizmo_handles)
            .add_system(gizmo_toolbar)
            .add_system(update_gizmo_visibility)
            .add_system(position_gizmos);
    }
}
