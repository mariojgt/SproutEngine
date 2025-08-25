// Project management system for Sprout Engine
// Handles project creation, loading, and settings

use bevy::prelude::*;
use bevy_egui::{egui, EguiContexts};
use serde::{Deserialize, Serialize};
use std::path::PathBuf;

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct ProjectSettings {
    pub name: String,
    pub version: String,
    pub description: String,
    pub author: String,
    pub engine_version: String,
    pub target_platforms: Vec<TargetPlatform>,
    pub build_settings: BuildSettings,
    pub physics_settings: PhysicsSettings,
    pub rendering_settings: RenderingSettings,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub enum TargetPlatform {
    Windows,
    MacOS,
    Linux,
    Web,
    Android,
    IOS,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct BuildSettings {
    pub optimization_level: OptimizationLevel,
    pub strip_debug_info: bool,
    pub compress_assets: bool,
    pub bundle_assets: bool,
}

#[derive(Serialize, Deserialize, Clone, Debug, PartialEq)]
pub enum OptimizationLevel {
    Debug,
    Release,
    ReleaseWithDebugInfo,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct PhysicsSettings {
    pub gravity: [f32; 3],
    pub timestep: f32,
    pub substeps: u32,
    pub default_material_friction: f32,
    pub default_material_restitution: f32,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct RenderingSettings {
    pub target_fps: u32,
    pub vsync: bool,
    pub msaa_samples: u32,
    pub shadow_quality: ShadowQuality,
    pub texture_quality: TextureQuality,
    pub post_processing: bool,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub enum ShadowQuality {
    Low,
    Medium,
    High,
    Ultra,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub enum TextureQuality {
    Low,
    Medium,
    High,
    Ultra,
}

impl Default for ProjectSettings {
    fn default() -> Self {
        Self {
            name: "New Sprout Project".to_string(),
            version: "1.0.0".to_string(),
            description: "A game created with Sprout Engine".to_string(),
            author: "Unknown".to_string(),
            engine_version: "0.1.0".to_string(),
            target_platforms: vec![TargetPlatform::Windows, TargetPlatform::MacOS, TargetPlatform::Linux],
            build_settings: BuildSettings {
                optimization_level: OptimizationLevel::Debug,
                strip_debug_info: false,
                compress_assets: true,
                bundle_assets: true,
            },
            physics_settings: PhysicsSettings {
                gravity: [0.0, -9.81, 0.0],
                timestep: 1.0 / 60.0,
                substeps: 4,
                default_material_friction: 0.7,
                default_material_restitution: 0.3,
            },
            rendering_settings: RenderingSettings {
                target_fps: 60,
                vsync: true,
                msaa_samples: 4,
                shadow_quality: ShadowQuality::High,
                texture_quality: TextureQuality::High,
                post_processing: true,
            },
        }
    }
}

#[derive(Resource)]
pub struct ProjectManager {
    pub current_project: Option<ProjectSettings>,
    pub project_path: Option<PathBuf>,
    pub recent_projects: Vec<PathBuf>,
    pub show_project_settings: bool,
    pub show_new_project_dialog: bool,
}

impl Default for ProjectManager {
    fn default() -> Self {
        Self {
            current_project: Some(ProjectSettings::default()),
            project_path: None,
            recent_projects: Vec::new(),
            show_project_settings: false,
            show_new_project_dialog: false,
        }
    }
}

pub fn project_menu(
    mut contexts: EguiContexts,
    mut project_manager: ResMut<ProjectManager>,
) {
    egui::TopBottomPanel::top("project_menu").show(contexts.ctx_mut(), |ui| {
        egui::menu::bar(ui, |ui| {
            ui.menu_button("Project", |ui| {
                if ui.button("New Project...").clicked() {
                    project_manager.show_new_project_dialog = true;
                    ui.close_menu();
                }

                if ui.button("Open Project...").clicked() {
                    // In a real implementation, this would open a file dialog
                    ui.close_menu();
                }

                ui.separator();

                if ui.button("Save Project").clicked() {
                    if let Some(project) = &project_manager.current_project {
                        save_project(project, &project_manager.project_path);
                    }
                    ui.close_menu();
                }

                if ui.button("Save Project As...").clicked() {
                    // In a real implementation, this would open a save file dialog
                    ui.close_menu();
                }

                ui.separator();

                if ui.button("Project Settings...").clicked() {
                    project_manager.show_project_settings = true;
                    ui.close_menu();
                }

                ui.separator();

                ui.label("Recent Projects:");
                for recent_path in &project_manager.recent_projects.clone() {
                    if let Some(file_name) = recent_path.file_name() {
                        if ui.button(file_name.to_string_lossy()).clicked() {
                            load_project(&mut project_manager, recent_path.clone());
                            ui.close_menu();
                        }
                    }
                }
            });

            ui.menu_button("Build", |ui| {
                if ui.button("Build Project").clicked() {
                    build_project(&project_manager);
                    ui.close_menu();
                }

                if ui.button("Build and Run").clicked() {
                    build_and_run_project(&project_manager);
                    ui.close_menu();
                }

                ui.separator();

                if ui.button("Package for Distribution").clicked() {
                    package_project(&project_manager);
                    ui.close_menu();
                }

                ui.separator();

                if ui.button("Build Settings...").clicked() {
                    project_manager.show_project_settings = true;
                    ui.close_menu();
                }
            });
        });
    });
}

pub fn new_project_dialog(
    mut contexts: EguiContexts,
    mut project_manager: ResMut<ProjectManager>,
) {
    if !project_manager.show_new_project_dialog {
        return;
    }

    let mut new_project = ProjectSettings::default();

    egui::Window::new("New Project")
        .collapsible(false)
        .resizable(false)
        .show(contexts.ctx_mut(), |ui| {
            ui.horizontal(|ui| {
                ui.label("Project Name:");
                ui.text_edit_singleline(&mut new_project.name);
            });

            ui.horizontal(|ui| {
                ui.label("Author:");
                ui.text_edit_singleline(&mut new_project.author);
            });

            ui.horizontal(|ui| {
                ui.label("Description:");
                ui.text_edit_multiline(&mut new_project.description);
            });

            ui.separator();

            ui.horizontal(|ui| {
                if ui.button("Create").clicked() {
                    project_manager.current_project = Some(new_project);
                    project_manager.show_new_project_dialog = false;
                }

                if ui.button("Cancel").clicked() {
                    project_manager.show_new_project_dialog = false;
                }
            });
        });
}

pub fn project_settings_dialog(
    mut contexts: EguiContexts,
    mut project_manager: ResMut<ProjectManager>,
) {
    if !project_manager.show_project_settings {
        return;
    }

    let project_path = project_manager.project_path.clone();
    let mut close_dialog = false;

    if let Some(project) = &mut project_manager.current_project {
        egui::Window::new("Project Settings")
            .collapsible(false)
            .resizable(true)
            .default_size([500.0, 600.0])
            .show(contexts.ctx_mut(), |ui| {
                egui::ScrollArea::vertical().show(ui, |ui| {
                    ui.collapsing("General", |ui| {
                        ui.horizontal(|ui| {
                            ui.label("Name:");
                            ui.text_edit_singleline(&mut project.name);
                        });

                        ui.horizontal(|ui| {
                            ui.label("Version:");
                            ui.text_edit_singleline(&mut project.version);
                        });

                        ui.horizontal(|ui| {
                            ui.label("Author:");
                            ui.text_edit_singleline(&mut project.author);
                        });

                        ui.horizontal(|ui| {
                            ui.label("Description:");
                            ui.text_edit_multiline(&mut project.description);
                        });
                    });

                    ui.collapsing("Build Settings", |ui| {
                        ui.horizontal(|ui| {
                            ui.label("Optimization:");
                            egui::ComboBox::from_label("")
                                .selected_text(format!("{:?}", project.build_settings.optimization_level))
                                .show_ui(ui, |ui| {
                                    ui.selectable_value(&mut project.build_settings.optimization_level, OptimizationLevel::Debug, "Debug");
                                    ui.selectable_value(&mut project.build_settings.optimization_level, OptimizationLevel::Release, "Release");
                                    ui.selectable_value(&mut project.build_settings.optimization_level, OptimizationLevel::ReleaseWithDebugInfo, "Release with Debug Info");
                                });
                        });

                        ui.checkbox(&mut project.build_settings.strip_debug_info, "Strip Debug Info");
                        ui.checkbox(&mut project.build_settings.compress_assets, "Compress Assets");
                        ui.checkbox(&mut project.build_settings.bundle_assets, "Bundle Assets");
                    });

                    ui.collapsing("Physics Settings", |ui| {
                        ui.horizontal(|ui| {
                            ui.label("Gravity:");
                            ui.add(egui::DragValue::new(&mut project.physics_settings.gravity[0]).speed(0.1));
                            ui.add(egui::DragValue::new(&mut project.physics_settings.gravity[1]).speed(0.1));
                            ui.add(egui::DragValue::new(&mut project.physics_settings.gravity[2]).speed(0.1));
                        });

                        ui.horizontal(|ui| {
                            ui.label("Timestep:");
                            ui.add(egui::DragValue::new(&mut project.physics_settings.timestep).speed(0.001).clamp_range(0.001..=0.1));
                        });

                        ui.horizontal(|ui| {
                            ui.label("Substeps:");
                            ui.add(egui::DragValue::new(&mut project.physics_settings.substeps).clamp_range(1..=16));
                        });
                    });

                    ui.collapsing("Rendering Settings", |ui| {
                        ui.horizontal(|ui| {
                            ui.label("Target FPS:");
                            ui.add(egui::DragValue::new(&mut project.rendering_settings.target_fps).clamp_range(30..=240));
                        });

                        ui.checkbox(&mut project.rendering_settings.vsync, "VSync");
                        ui.checkbox(&mut project.rendering_settings.post_processing, "Post Processing");

                        ui.horizontal(|ui| {
                            ui.label("MSAA Samples:");
                            egui::ComboBox::from_label("")
                                .selected_text(project.rendering_settings.msaa_samples.to_string())
                                .show_ui(ui, |ui| {
                                    ui.selectable_value(&mut project.rendering_settings.msaa_samples, 1, "1");
                                    ui.selectable_value(&mut project.rendering_settings.msaa_samples, 2, "2");
                                    ui.selectable_value(&mut project.rendering_settings.msaa_samples, 4, "4");
                                    ui.selectable_value(&mut project.rendering_settings.msaa_samples, 8, "8");
                                });
                        });
                    });
                });

                ui.separator();

                ui.horizontal(|ui| {
                    if ui.button("Save").clicked() {
                        save_project(project, &project_path);
                        close_dialog = true;
                    }

                    if ui.button("Cancel").clicked() {
                        close_dialog = true;
                    }
                });
            });
    }

    // Handle close dialog outside the UI closure to avoid borrow checker issues
    if close_dialog || contexts.ctx_mut().input(|i| i.key_pressed(egui::Key::Escape)) {
        project_manager.show_project_settings = false;
    }
}

fn save_project(project: &ProjectSettings, path: &Option<PathBuf>) {
    if let Some(project_path) = path {
        let project_file = project_path.join("project.sprout");
        if let Ok(json) = serde_json::to_string_pretty(project) {
            let _ = std::fs::write(project_file, json);
        }
    }
}

fn load_project(project_manager: &mut ProjectManager, path: PathBuf) {
    let project_file = path.join("project.sprout");
    if let Ok(content) = std::fs::read_to_string(project_file) {
        if let Ok(project) = serde_json::from_str::<ProjectSettings>(&content) {
            project_manager.current_project = Some(project);
            project_manager.project_path = Some(path.clone());

            // Add to recent projects
            if !project_manager.recent_projects.contains(&path) {
                project_manager.recent_projects.insert(0, path);
                if project_manager.recent_projects.len() > 10 {
                    project_manager.recent_projects.truncate(10);
                }
            }
        }
    }
}

fn build_project(_project_manager: &ProjectManager) {
    // In a real implementation, this would invoke cargo build
    println!("Building project...");
}

fn build_and_run_project(_project_manager: &ProjectManager) {
    // In a real implementation, this would invoke cargo run
    println!("Building and running project...");
}

fn package_project(_project_manager: &ProjectManager) {
    // In a real implementation, this would package the project for distribution
    println!("Packaging project...");
}

pub struct ProjectPlugin;

impl Plugin for ProjectPlugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<ProjectManager>()
            .add_system(project_menu)
            .add_system(new_project_dialog)
            .add_system(project_settings_dialog);
    }
}
