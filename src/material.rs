// Enhanced material system for Sprout Engine
// Provides PBR material editing and management capabilities

use bevy::prelude::*;
use bevy_egui::{egui, EguiContexts};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct SproutMaterial {
    pub name: String,
    pub albedo_color: [f32; 4],
    pub metallic: f32,
    pub roughness: f32,
    pub emission: [f32; 3],
    pub emission_strength: f32,
    pub normal_strength: f32,
    pub albedo_texture: Option<String>,
    pub normal_texture: Option<String>,
    pub metallic_roughness_texture: Option<String>,
    pub emission_texture: Option<String>,
    pub occlusion_texture: Option<String>,
}

impl Default for SproutMaterial {
    fn default() -> Self {
        Self {
            name: "New Material".to_string(),
            albedo_color: [0.8, 0.8, 0.8, 1.0],
            metallic: 0.0,
            roughness: 0.5,
            emission: [0.0, 0.0, 0.0],
            emission_strength: 1.0,
            normal_strength: 1.0,
            albedo_texture: None,
            normal_texture: None,
            metallic_roughness_texture: None,
            emission_texture: None,
            occlusion_texture: None,
        }
    }
}

#[derive(Resource, Serialize, Deserialize)]
pub struct MaterialLibrary {
    pub materials: HashMap<String, SproutMaterial>,
    pub selected_material: Option<String>,
}

impl Default for MaterialLibrary {
    fn default() -> Self {
        let mut materials = HashMap::new();

        // Add some default materials
        materials.insert("Default".to_string(), SproutMaterial {
            name: "Default".to_string(),
            ..Default::default()
        });

        materials.insert("Metal".to_string(), SproutMaterial {
            name: "Metal".to_string(),
            albedo_color: [0.7, 0.7, 0.7, 1.0],
            metallic: 1.0,
            roughness: 0.2,
            ..Default::default()
        });

        materials.insert("Plastic".to_string(), SproutMaterial {
            name: "Plastic".to_string(),
            albedo_color: [0.1, 0.5, 0.9, 1.0],
            metallic: 0.0,
            roughness: 0.8,
            ..Default::default()
        });

        Self {
            materials,
            selected_material: Some("Default".to_string()),
        }
    }
}

pub fn material_editor_ui(
    mut contexts: EguiContexts,
    mut material_library: ResMut<MaterialLibrary>,
    asset_server: Res<AssetServer>,
    mut bevy_materials: ResMut<Assets<StandardMaterial>>,
) {
    egui::Window::new("Material Editor")
        .default_open(true)
        .resizable(true)
        .default_size([400.0, 600.0])
        .show(contexts.ctx_mut(), |ui| {

        ui.horizontal(|ui| {
            ui.label("Material:");

            egui::ComboBox::from_label("")
                .selected_text(material_library.selected_material.as_ref().unwrap_or(&"None".to_string()))
                .show_ui(ui, |ui| {
                    let material_names: Vec<String> = material_library.materials.keys().cloned().collect();
                    for material_name in material_names {
                        ui.selectable_value(&mut material_library.selected_material, Some(material_name.clone()), material_name);
                    }
                });

            if ui.button("New").clicked() {
                let new_name = format!("Material_{}", material_library.materials.len());
                material_library.materials.insert(new_name.clone(), SproutMaterial {
                    name: new_name.clone(),
                    ..Default::default()
                });
                material_library.selected_material = Some(new_name);
            }

            if ui.button("Delete").clicked() {
                if let Some(selected) = material_library.selected_material.clone() {
                    if selected != "Default" { // Don't delete the default material
                        material_library.materials.remove(&selected);
                        material_library.selected_material = Some("Default".to_string());
                    }
                }
            }
        });

        ui.separator();

        if let Some(selected_name) = &material_library.selected_material.clone() {
            if let Some(material) = material_library.materials.get_mut(selected_name) {
                ui.heading("Properties");

                ui.horizontal(|ui| {
                    ui.label("Name:");
                    ui.text_edit_singleline(&mut material.name);
                });

                ui.separator();

                // Albedo color
                ui.horizontal(|ui| {
                    ui.label("Albedo:");
                    ui.color_edit_button_rgba_unmultiplied(&mut material.albedo_color);
                });

                // Metallic
                ui.horizontal(|ui| {
                    ui.label("Metallic:");
                    ui.add(egui::Slider::new(&mut material.metallic, 0.0..=1.0));
                });

                // Roughness
                ui.horizontal(|ui| {
                    ui.label("Roughness:");
                    ui.add(egui::Slider::new(&mut material.roughness, 0.0..=1.0));
                });

                ui.separator();

                // Emission
                ui.horizontal(|ui| {
                    ui.label("Emission:");
                    ui.color_edit_button_rgb(&mut material.emission);
                });

                ui.horizontal(|ui| {
                    ui.label("Emission Strength:");
                    ui.add(egui::Slider::new(&mut material.emission_strength, 0.0..=10.0));
                });

                ui.separator();

                // Normal strength
                ui.horizontal(|ui| {
                    ui.label("Normal Strength:");
                    ui.add(egui::Slider::new(&mut material.normal_strength, 0.0..=2.0));
                });

                ui.separator();
                ui.heading("Textures");

                // Texture slots
                texture_slot_ui(ui, "Albedo", &mut material.albedo_texture);
                texture_slot_ui(ui, "Normal", &mut material.normal_texture);
                texture_slot_ui(ui, "Metallic/Roughness", &mut material.metallic_roughness_texture);
                texture_slot_ui(ui, "Emission", &mut material.emission_texture);
                texture_slot_ui(ui, "Occlusion", &mut material.occlusion_texture);

                ui.separator();

                if ui.button("Apply to Selected Objects").clicked() {
                    // This would apply the material to selected objects
                    // For now, we'll just create a Bevy StandardMaterial
                    let bevy_material = create_bevy_material(material, &asset_server);
                    bevy_materials.add(bevy_material);
                }
            }
        }
    });
}

fn texture_slot_ui(ui: &mut egui::Ui, label: &str, texture_path: &mut Option<String>) {
    ui.horizontal(|ui| {
        ui.label(label);

        let display_text = texture_path.as_ref()
            .map(|path| path.split('/').last().unwrap_or(path))
            .unwrap_or("None");

        ui.text_edit_singleline(&mut display_text.to_string());

        if ui.button("Browse").clicked() {
            // In a real implementation, this would open a file dialog
            // For now, we'll use a placeholder
            *texture_path = Some("assets/textures/placeholder.png".to_string());
        }

        if ui.button("Clear").clicked() {
            *texture_path = None;
        }
    });
}

fn create_bevy_material(sprout_material: &SproutMaterial, asset_server: &AssetServer) -> StandardMaterial {
    let mut material = StandardMaterial {
        base_color: Color::rgba(
            sprout_material.albedo_color[0],
            sprout_material.albedo_color[1],
            sprout_material.albedo_color[2],
            sprout_material.albedo_color[3],
        ),
        metallic: sprout_material.metallic,
        perceptual_roughness: sprout_material.roughness,
        emissive: Color::rgb(
            sprout_material.emission[0] * sprout_material.emission_strength,
            sprout_material.emission[1] * sprout_material.emission_strength,
            sprout_material.emission[2] * sprout_material.emission_strength,
        ),
        ..default()
    };

    // Load textures if specified
    if let Some(albedo_path) = &sprout_material.albedo_texture {
        material.base_color_texture = Some(asset_server.load(albedo_path));
    }

    if let Some(normal_path) = &sprout_material.normal_texture {
        material.normal_map_texture = Some(asset_server.load(normal_path));
    }

    if let Some(metallic_roughness_path) = &sprout_material.metallic_roughness_texture {
        material.metallic_roughness_texture = Some(asset_server.load(metallic_roughness_path));
    }

    if let Some(emission_path) = &sprout_material.emission_texture {
        material.emissive_texture = Some(asset_server.load(emission_path));
    }

    if let Some(occlusion_path) = &sprout_material.occlusion_texture {
        material.occlusion_texture = Some(asset_server.load(occlusion_path));
    }

    material
}

pub fn save_material_library(material_library: &MaterialLibrary, path: &str) -> Result<(), Box<dyn std::error::Error>> {
    let json = serde_json::to_string_pretty(material_library)?;
    std::fs::write(path, json)?;
    Ok(())
}

pub fn load_material_library(path: &str) -> Result<MaterialLibrary, Box<dyn std::error::Error>> {
    let json = std::fs::read_to_string(path)?;
    let library = serde_json::from_str(&json)?;
    Ok(library)
}

pub struct MaterialPlugin;

impl Plugin for MaterialPlugin {
    fn build(&self, app: &mut App) {
        app
            .init_resource::<MaterialLibrary>()
            .add_system(material_editor_ui);
    }
}
