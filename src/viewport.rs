// Enhanced viewport system for Sprout Engine
// This module provides multiple viewport support and advanced camera controls

use bevy::prelude::*;
use bevy_egui::{egui, EguiContexts};

#[derive(Component)]
pub struct EditorCamera {
    pub viewport_type: ViewportType,
    pub is_active: bool,
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum ViewportType {
    Perspective,
    Top,
    Front,
    Right,
    Custom,
}

#[derive(Resource)]
pub struct ViewportManager {
    pub active_viewport: ViewportType,
    pub show_grid: bool,
    pub grid_size: f32,
    pub show_gizmos: bool,
    pub camera_speed: f32,
}

impl Default for ViewportManager {
    fn default() -> Self {
        Self {
            active_viewport: ViewportType::Perspective,
            show_grid: true,
            grid_size: 1.0,
            show_gizmos: true,
            camera_speed: 5.0,
        }
    }
}

pub fn setup_viewports(mut commands: Commands) {
    // Main perspective camera
    commands.spawn((
        Name::new("PerspectiveCamera"),
        Camera3dBundle {
            transform: Transform::from_xyz(0.0, 5.0, 10.0).looking_at(Vec3::ZERO, Vec3::Y),
            camera: Camera {
                order: 0,
                ..default()
            },
            ..default()
        },
        EditorCamera {
            viewport_type: ViewportType::Perspective,
            is_active: true,
        },
    ));

    // Top orthographic camera
    commands.spawn((
        Name::new("TopCamera"),
        Camera3dBundle {
            projection: Projection::Orthographic(OrthographicProjection {
                scale: 10.0,
                ..default()
            }),
            transform: Transform::from_xyz(0.0, 20.0, 0.0).looking_at(Vec3::ZERO, Vec3::Z),
            camera: Camera {
                order: 1,
                is_active: false,
                ..default()
            },
            ..default()
        },
        EditorCamera {
            viewport_type: ViewportType::Top,
            is_active: false,
        },
    ));

    // Front orthographic camera
    commands.spawn((
        Name::new("FrontCamera"),
        Camera3dBundle {
            projection: Projection::Orthographic(OrthographicProjection {
                scale: 10.0,
                ..default()
            }),
            transform: Transform::from_xyz(0.0, 0.0, 20.0).looking_at(Vec3::ZERO, Vec3::Y),
            camera: Camera {
                order: 2,
                is_active: false,
                ..default()
            },
            ..default()
        },
        EditorCamera {
            viewport_type: ViewportType::Front,
            is_active: false,
        },
    ));

    // Right orthographic camera
    commands.spawn((
        Name::new("RightCamera"),
        Camera3dBundle {
            projection: Projection::Orthographic(OrthographicProjection {
                scale: 10.0,
                ..default()
            }),
            transform: Transform::from_xyz(20.0, 0.0, 0.0).looking_at(Vec3::ZERO, Vec3::Y),
            camera: Camera {
                order: 3,
                is_active: false,
                ..default()
            },
            ..default()
        },
        EditorCamera {
            viewport_type: ViewportType::Right,
            is_active: false,
        },
    ));
}

pub fn viewport_ui(
    mut contexts: EguiContexts,
    mut viewport_manager: ResMut<ViewportManager>,
    mut cameras: Query<(&mut Camera, &mut EditorCamera)>,
) {
    egui::TopBottomPanel::top("viewport_toolbar").show(contexts.ctx_mut(), |ui| {
        ui.horizontal(|ui| {
            ui.label("Viewport:");

            if ui.selectable_label(viewport_manager.active_viewport == ViewportType::Perspective, "Perspective").clicked() {
                switch_viewport(&mut viewport_manager, &mut cameras, ViewportType::Perspective);
            }

            if ui.selectable_label(viewport_manager.active_viewport == ViewportType::Top, "Top").clicked() {
                switch_viewport(&mut viewport_manager, &mut cameras, ViewportType::Top);
            }

            if ui.selectable_label(viewport_manager.active_viewport == ViewportType::Front, "Front").clicked() {
                switch_viewport(&mut viewport_manager, &mut cameras, ViewportType::Front);
            }

            if ui.selectable_label(viewport_manager.active_viewport == ViewportType::Right, "Right").clicked() {
                switch_viewport(&mut viewport_manager, &mut cameras, ViewportType::Right);
            }

            ui.separator();

            ui.checkbox(&mut viewport_manager.show_grid, "Grid");
            ui.checkbox(&mut viewport_manager.show_gizmos, "Gizmos");

            ui.separator();

            ui.label("Grid Size:");
            ui.add(egui::DragValue::new(&mut viewport_manager.grid_size).speed(0.1).clamp_range(0.1..=10.0));

            ui.separator();

            ui.label("Speed:");
            ui.add(egui::DragValue::new(&mut viewport_manager.camera_speed).speed(0.1).clamp_range(0.1..=20.0));
        });
    });
}

fn switch_viewport(
    viewport_manager: &mut ViewportManager,
    cameras: &mut Query<(&mut Camera, &mut EditorCamera)>,
    new_viewport: ViewportType,
) {
    viewport_manager.active_viewport = new_viewport;

    for (mut camera, mut editor_camera) in cameras.iter_mut() {
        let should_be_active = editor_camera.viewport_type == new_viewport;
        camera.is_active = should_be_active;
        editor_camera.is_active = should_be_active;
    }
}

pub fn draw_grid(
    viewport_manager: Res<ViewportManager>,
) {
    if !viewport_manager.show_grid {
        return;
    }

    // Note: In Bevy 0.10, gizmo system is different
    // This is a placeholder for grid rendering
    // In a real implementation, we'd use a custom mesh or shader
}

pub struct ViewportPlugin;

impl Plugin for ViewportPlugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<ViewportManager>()
            .add_startup_system(setup_viewports)
            .add_system(viewport_ui)
            .add_system(draw_grid);
    }
}
