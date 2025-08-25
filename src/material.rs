// Enhanced material system for Sprout Engine
// Provides PBR material editing and management capabilities with visual node editor

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
    pub material_graph: MaterialGraph,  // Visual node graph for the material
}

// Node-based material system structures
#[derive(Serialize, Deserialize, Clone, Debug, PartialEq)]
pub enum MaterialNodeType {
    // Input nodes
    TextureSample,
    ColorConstant,
    ScalarConstant,
    Vector3Constant,

    // PBR-specific nodes
    Fresnel,
    NormalMap,
    DetailNormal,

    // Noise and patterns
    NoiseTexture,
    CheckerPattern,

    // Utility nodes
    ColorMix,
    Clamp,
    Remap,

    // Math nodes
    Multiply,
    Add,
    Subtract,
    Lerp,
    Power,
    OneMinus,

    // Output node (there's always one)
    MaterialOutput,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct MaterialNode {
    pub id: u32,
    pub node_type: MaterialNodeType,
    pub position: [f32; 2],
    pub inputs: HashMap<String, MaterialValue>,
    pub name: String,
}

#[derive(Serialize, Deserialize, Clone, Debug, PartialEq)]
pub enum MaterialValue {
    Color([f32; 4]),
    Scalar(f32),
    Vector3([f32; 3]),
    Texture(String),
    Connection { node_id: u32, output_name: String },
}

#[derive(Clone, Debug, PartialEq)]
pub enum SocketType {
    Color,
    Scalar,
    Vector3,
    Texture,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct MaterialConnection {
    pub from_node: u32,
    pub from_output: String,
    pub to_node: u32,
    pub to_input: String,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct MaterialGraph {
    pub nodes: Vec<MaterialNode>,
    pub connections: Vec<MaterialConnection>,
    pub next_node_id: u32,
}

impl Default for MaterialGraph {
    fn default() -> Self {
        let mut graph = Self {
            nodes: Vec::new(),
            connections: Vec::new(),
            next_node_id: 2,
        };

        // Always start with a Material Output node
        graph.nodes.push(MaterialNode {
            id: 1,
            node_type: MaterialNodeType::MaterialOutput,
            position: [400.0, 200.0],
            inputs: HashMap::from([
                ("Base Color".to_string(), MaterialValue::Color([0.8, 0.8, 0.8, 1.0])),
                ("Metallic".to_string(), MaterialValue::Scalar(0.0)),
                ("Roughness".to_string(), MaterialValue::Scalar(0.5)),
                ("Emission".to_string(), MaterialValue::Color([0.0, 0.0, 0.0, 1.0])),
            ]),
            name: "Material Output".to_string(),
        });

        graph
    }
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
            material_graph: MaterialGraph::default(),
        }
    }
}

#[derive(Resource, Serialize, Deserialize)]
pub struct MaterialLibrary {
    pub materials: HashMap<String, SproutMaterial>,
    pub selected_material: Option<String>,
}

#[derive(Resource)]
pub struct MaterialEditorState {
    pub show_node_editor: bool,
    pub selected_node: Option<u32>,
    pub dragging_node: Option<u32>,
    pub drag_offset: [f32; 2],
    pub creating_connection: Option<(u32, String)>, // (from_node_id, output_name)
}

// Helper functions for connection validation
fn get_input_socket_type(node_type: &MaterialNodeType, input_name: &str) -> Option<SocketType> {
    match (node_type, input_name) {
        // Specific node type matches first
        (MaterialNodeType::Lerp, "A" | "B") => Some(SocketType::Color),
        (MaterialNodeType::MaterialOutput, "Base Color" | "Emission") => Some(SocketType::Color),
        (MaterialNodeType::MaterialOutput, "Metallic" | "Roughness") => Some(SocketType::Scalar),

        // General color inputs
        (_, "Base Color" | "Color" | "Color A" | "Color B" | "Color1" | "Color2") => Some(SocketType::Color),
        (MaterialNodeType::Multiply | MaterialNodeType::Add | MaterialNodeType::Subtract, "A" | "B") => Some(SocketType::Scalar), // Math nodes default to scalar

        // Scalar inputs
        (_, "Metallic" | "Roughness" | "Value" | "Alpha" | "Factor" | "Strength" | "Scale" | "Detail" | "Distortion" | "IOR" | "Min" | "Max" | "From Min" | "From Max" | "To Min" | "To Max" | "Exponent" | "Input" | "Base") => Some(SocketType::Scalar),

        // Vector3 inputs
        (_, "Vector" | "Normal" | "UV" | "Base Normal" | "Detail Normal") => Some(SocketType::Vector3),

        // Texture inputs
        (_, "Texture" | "Normal Texture") => Some(SocketType::Texture),

        _ => Some(SocketType::Scalar), // Default to scalar for math operations
    }
}

fn get_output_socket_type(node_type: &MaterialNodeType, output_name: &str) -> Option<SocketType> {
    match (node_type, output_name) {
        // Color outputs
        (MaterialNodeType::ColorConstant, "Color") => Some(SocketType::Color),
        (MaterialNodeType::TextureSample, "RGB") => Some(SocketType::Color),
        (MaterialNodeType::CheckerPattern, "Pattern") => Some(SocketType::Color),
        (MaterialNodeType::ColorMix, "Color") => Some(SocketType::Color),

        // Scalar outputs
        (MaterialNodeType::ScalarConstant, "Value") => Some(SocketType::Scalar),
        (MaterialNodeType::TextureSample, "Alpha") => Some(SocketType::Scalar),
        (MaterialNodeType::NoiseTexture, "Noise") => Some(SocketType::Scalar),
        (MaterialNodeType::Fresnel, "Fresnel") => Some(SocketType::Scalar),
        (MaterialNodeType::Multiply | MaterialNodeType::Add | MaterialNodeType::Subtract | MaterialNodeType::Power | MaterialNodeType::OneMinus | MaterialNodeType::Clamp | MaterialNodeType::Remap, "Result") => Some(SocketType::Scalar),
        (MaterialNodeType::Lerp, "Result") => Some(SocketType::Color), // Lerp typically outputs color

        // Vector3 outputs
        (MaterialNodeType::Vector3Constant, "Vector") => Some(SocketType::Vector3),
        (MaterialNodeType::NormalMap | MaterialNodeType::DetailNormal, "Normal") => Some(SocketType::Vector3),

        _ => None,
    }
}

fn is_connection_valid(from_node_type: &MaterialNodeType, from_output: &str, to_node_type: &MaterialNodeType, to_input: &str) -> bool {
    let from_type = get_output_socket_type(from_node_type, from_output);
    let to_type = get_input_socket_type(to_node_type, to_input);

    match (from_type, to_type) {
        (Some(from), Some(to)) => {
            // Exact type match
            if from == to {
                return true;
            }

            // Allow some flexible connections
            match (from, to) {
                // Scalar can connect to color (will be used for all channels)
                (SocketType::Scalar, SocketType::Color) => true,
                // Color can connect to scalar (will use luminance)
                (SocketType::Color, SocketType::Scalar) => true,
                _ => false,
            }
        }
        _ => false,
    }
}

impl Default for MaterialEditorState {
    fn default() -> Self {
        Self {
            show_node_editor: false,
            selected_node: None,
            dragging_node: None,
            drag_offset: [0.0, 0.0],
            creating_connection: None,
        }
    }
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
    mut editor_state: ResMut<MaterialEditorState>,
    _asset_server: Res<AssetServer>,
    _bevy_materials: ResMut<Assets<StandardMaterial>>,
    keys: Res<Input<KeyCode>>,
) {
    // Material Browser Window
    egui::Window::new("Material Browser")
        .default_open(true)
        .resizable(true)
        .default_size([300.0, 400.0])
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

        // Node Editor Button
        if ui.button("Open Node Editor").clicked() {
            editor_state.show_node_editor = true;
        }

        ui.separator();

        // Material Properties (read-only, computed from graph)
        if let Some(selected_name) = &material_library.selected_material.clone() {
            if let Some(material) = material_library.materials.get(selected_name) {
                ui.label("Material Properties (computed from graph):");

                ui.horizontal(|ui| {
                    ui.label("Base Color:");
                    let color = material.albedo_color;
                    ui.colored_label(
                        egui::Color32::from_rgba_premultiplied(
                            (color[0] * 255.0) as u8,
                            (color[1] * 255.0) as u8,
                            (color[2] * 255.0) as u8,
                            255,
                        ),
                        "█████"
                    );
                });

                ui.label(format!("Metallic: {:.2}", material.metallic));
                ui.label(format!("Roughness: {:.2}", material.roughness));
                ui.label(format!("Nodes in graph: {}", material.material_graph.nodes.len()));
            }
        }
    });

    // Visual Node Editor Window
    if editor_state.show_node_editor {
        material_node_editor_ui(contexts.ctx_mut(), &mut material_library, &mut editor_state, &keys);
    }
}

pub fn material_node_editor_ui(
    ctx: &mut egui::Context,
    material_library: &mut MaterialLibrary,
    editor_state: &mut MaterialEditorState,
    keys: &Res<Input<KeyCode>>,
) {
    egui::Window::new("Material Node Editor")
        .default_open(true)
        .resizable(true)
        .default_size([800.0, 600.0])
        .show(ctx, |ui| {

            if ui.button("Close Node Editor").clicked() {
                editor_state.show_node_editor = false;
                return;
            }

            if let Some(selected_name) = &material_library.selected_material.clone() {
                if let Some(material) = material_library.materials.get_mut(selected_name) {

                    // Handle keyboard input for node deletion
                    if (keys.just_pressed(KeyCode::Delete) || keys.just_pressed(KeyCode::Back)) && !ctx.wants_keyboard_input() {
                        if let Some(selected_node_id) = editor_state.selected_node {
                            // Don't allow deletion of Material Output node
                            if let Some(node) = material.material_graph.nodes.iter().find(|n| n.id == selected_node_id) {
                                if node.node_type != MaterialNodeType::MaterialOutput {
                                    // Remove the node
                                    material.material_graph.nodes.retain(|n| n.id != selected_node_id);

                                    // Remove all connections involving this node
                                    material.material_graph.connections.retain(|c|
                                        c.from_node != selected_node_id && c.to_node != selected_node_id
                                    );

                                    // Clear selection
                                    editor_state.selected_node = None;

                                    println!("Deleted node {}", selected_node_id);
                                } else {
                                    println!("Cannot delete Material Output node");
                                }
                            }
                        }
                    }

                    ui.separator();

                    // Display current selection info
                    if let Some(selected_id) = editor_state.selected_node {
                        ui.label(format!("Selected Node: {} (Press Delete to remove)", selected_id));
                    } else {
                        ui.label("No node selected (Click on a node to select it)");
                    }

                    ui.separator();

                    // Add Node buttons
                    ui.vertical(|ui| {
                        ui.label("Add Nodes:");

                        ui.horizontal(|ui| {
                            ui.label("Input Nodes:");
                            if ui.button("Color").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::ColorConstant, "Color");
                            }
                            if ui.button("Scalar").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::ScalarConstant, "Scalar");
                            }
                            if ui.button("Vector3").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Vector3Constant, "Vector3");
                            }
                            if ui.button("Texture").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::TextureSample, "Texture");
                            }
                        });

                        ui.horizontal(|ui| {
                            ui.label("PBR Nodes:");
                            if ui.button("Fresnel").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Fresnel, "Fresnel");
                            }
                            if ui.button("Normal Map").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::NormalMap, "Normal Map");
                            }
                            if ui.button("Detail Normal").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::DetailNormal, "Detail Normal");
                            }
                        });

                        ui.horizontal(|ui| {
                            ui.label("Patterns:");
                            if ui.button("Noise").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::NoiseTexture, "Noise");
                            }
                            if ui.button("Checker").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::CheckerPattern, "Checker");
                            }
                        });

                        ui.horizontal(|ui| {
                            ui.label("Math Nodes:");
                            if ui.button("Multiply").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Multiply, "Multiply");
                            }
                            if ui.button("Add").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Add, "Add");
                            }
                            if ui.button("Subtract").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Subtract, "Subtract");
                            }
                            if ui.button("Lerp").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Lerp, "Lerp");
                            }
                            if ui.button("Power").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Power, "Power");
                            }
                        });

                        ui.horizontal(|ui| {
                            ui.label("Utility:");
                            if ui.button("Color Mix").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::ColorMix, "Color Mix");
                            }
                            if ui.button("Clamp").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Clamp, "Clamp");
                            }
                            if ui.button("Remap").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::Remap, "Remap");
                            }
                            if ui.button("One Minus").clicked() {
                                add_node(&mut material.material_graph, MaterialNodeType::OneMinus, "One Minus");
                            }
                        });
                    });

                    ui.separator();

                    // Node canvas
                    let response = ui.allocate_response(
                        egui::vec2(ui.available_width(), 400.0),
                        egui::Sense::click_and_drag()
                    );

                    let painter = ui.painter_at(response.rect);

                    // Draw background grid
                    draw_grid(&painter, response.rect);

                    // Draw connections first (so they appear behind nodes)
                    for connection in &material.material_graph.connections {
                        draw_connection(&painter, &material.material_graph, connection, response.rect);
                    }

                    // Draw nodes
                    let graph_clone = material.material_graph.clone(); // Clone for reference during drawing
                    for node in &mut material.material_graph.nodes {
                        draw_node(
                            &painter,
                            ui,
                            node,
                            response.rect,
                            editor_state,
                            &response,
                            &graph_clone
                        );
                    }

                    // Draw preview connection line if creating a connection
                    if let Some((from_node_id, from_output)) = &editor_state.creating_connection {
                        if let Some(pointer_pos) = response.hover_pos() {
                            if let Some(from_node) = material.material_graph.nodes.iter().find(|n| n.id == *from_node_id) {
                                let from_pos = get_output_socket_pos(from_node, from_output, response.rect);
                                draw_preview_connection(&painter, from_pos, pointer_pos);
                            }
                        }
                    }

                    // Handle node interactions - removed ctx parameter to fix borrowing issue
                    handle_node_interactions(
                        &mut material.material_graph,
                        editor_state,
                        &response,
                        response.rect,
                    );

                    // Compile material from graph
                    compile_material_from_graph(material);
                }
            }
        });
}

// Helper functions for the node editor

fn add_node(graph: &mut MaterialGraph, node_type: MaterialNodeType, name: &str) {
    let node = MaterialNode {
        id: graph.next_node_id,
        node_type: node_type.clone(),
        position: [100.0, 100.0],
        inputs: get_default_inputs_for_node_type(&node_type),
        name: format!("{} {}", name, graph.next_node_id),
    };

    graph.nodes.push(node);
    graph.next_node_id += 1;
}

fn get_default_inputs_for_node_type(node_type: &MaterialNodeType) -> HashMap<String, MaterialValue> {
    match node_type {
        MaterialNodeType::ColorConstant => {
            HashMap::from([
                ("Color".to_string(), MaterialValue::Color([1.0, 1.0, 1.0, 1.0])),
            ])
        },
        MaterialNodeType::ScalarConstant => {
            HashMap::from([
                ("Value".to_string(), MaterialValue::Scalar(1.0)),
            ])
        },
        MaterialNodeType::Vector3Constant => {
            HashMap::from([
                ("Vector".to_string(), MaterialValue::Vector3([1.0, 1.0, 1.0])),
            ])
        },
        MaterialNodeType::TextureSample => {
            HashMap::from([
                ("Texture".to_string(), MaterialValue::Texture("".to_string())),
                ("UV".to_string(), MaterialValue::Vector3([0.0, 0.0, 0.0])),
            ])
        },
        MaterialNodeType::Fresnel => {
            HashMap::from([
                ("IOR".to_string(), MaterialValue::Scalar(1.45)),
                ("Normal".to_string(), MaterialValue::Vector3([0.0, 0.0, 1.0])),
            ])
        },
        MaterialNodeType::NormalMap => {
            HashMap::from([
                ("Normal Texture".to_string(), MaterialValue::Texture("".to_string())),
                ("Strength".to_string(), MaterialValue::Scalar(1.0)),
            ])
        },
        MaterialNodeType::DetailNormal => {
            HashMap::from([
                ("Base Normal".to_string(), MaterialValue::Vector3([0.0, 0.0, 1.0])),
                ("Detail Normal".to_string(), MaterialValue::Vector3([0.0, 0.0, 1.0])),
                ("Strength".to_string(), MaterialValue::Scalar(1.0)),
            ])
        },
        MaterialNodeType::NoiseTexture => {
            HashMap::from([
                ("Scale".to_string(), MaterialValue::Scalar(5.0)),
                ("Detail".to_string(), MaterialValue::Scalar(2.0)),
                ("Distortion".to_string(), MaterialValue::Scalar(0.0)),
            ])
        },
        MaterialNodeType::CheckerPattern => {
            HashMap::from([
                ("Color1".to_string(), MaterialValue::Color([1.0, 1.0, 1.0, 1.0])),
                ("Color2".to_string(), MaterialValue::Color([0.0, 0.0, 0.0, 1.0])),
                ("Scale".to_string(), MaterialValue::Scalar(8.0)),
            ])
        },
        MaterialNodeType::ColorMix => {
            HashMap::from([
                ("Color A".to_string(), MaterialValue::Color([1.0, 0.0, 0.0, 1.0])),
                ("Color B".to_string(), MaterialValue::Color([0.0, 1.0, 0.0, 1.0])),
                ("Factor".to_string(), MaterialValue::Scalar(0.5)),
            ])
        },
        MaterialNodeType::Clamp => {
            HashMap::from([
                ("Value".to_string(), MaterialValue::Scalar(0.5)),
                ("Min".to_string(), MaterialValue::Scalar(0.0)),
                ("Max".to_string(), MaterialValue::Scalar(1.0)),
            ])
        },
        MaterialNodeType::Remap => {
            HashMap::from([
                ("Value".to_string(), MaterialValue::Scalar(0.5)),
                ("From Min".to_string(), MaterialValue::Scalar(0.0)),
                ("From Max".to_string(), MaterialValue::Scalar(1.0)),
                ("To Min".to_string(), MaterialValue::Scalar(0.0)),
                ("To Max".to_string(), MaterialValue::Scalar(1.0)),
            ])
        },
        MaterialNodeType::Multiply => {
            HashMap::from([
                ("A".to_string(), MaterialValue::Scalar(1.0)),
                ("B".to_string(), MaterialValue::Scalar(1.0)),
            ])
        },
        MaterialNodeType::Add => {
            HashMap::from([
                ("A".to_string(), MaterialValue::Scalar(0.0)),
                ("B".to_string(), MaterialValue::Scalar(0.0)),
            ])
        },
        MaterialNodeType::Subtract => {
            HashMap::from([
                ("A".to_string(), MaterialValue::Scalar(1.0)),
                ("B".to_string(), MaterialValue::Scalar(0.0)),
            ])
        },
        MaterialNodeType::Lerp => {
            HashMap::from([
                ("A".to_string(), MaterialValue::Color([0.0, 0.0, 0.0, 1.0])),
                ("B".to_string(), MaterialValue::Color([1.0, 1.0, 1.0, 1.0])),
                ("Alpha".to_string(), MaterialValue::Scalar(0.5)),
            ])
        },
        MaterialNodeType::Power => {
            HashMap::from([
                ("Base".to_string(), MaterialValue::Scalar(0.5)),
                ("Exponent".to_string(), MaterialValue::Scalar(2.0)),
            ])
        },
        MaterialNodeType::OneMinus => {
            HashMap::from([
                ("Input".to_string(), MaterialValue::Scalar(0.5)),
            ])
        },
        MaterialNodeType::MaterialOutput => {
            HashMap::from([
                ("Base Color".to_string(), MaterialValue::Color([0.8, 0.8, 0.8, 1.0])),
                ("Metallic".to_string(), MaterialValue::Scalar(0.0)),
                ("Roughness".to_string(), MaterialValue::Scalar(0.5)),
                ("Emission".to_string(), MaterialValue::Color([0.0, 0.0, 0.0, 1.0])),
            ])
        },
    }
}

fn draw_grid(painter: &egui::Painter, rect: egui::Rect) {
    let grid_size = 20.0;
    let color = egui::Color32::from_rgba_premultiplied(50, 50, 50, 255);

    // Vertical lines
    let mut x = rect.left();
    while x < rect.right() {
        painter.line_segment(
            [egui::pos2(x, rect.top()), egui::pos2(x, rect.bottom())],
            egui::Stroke::new(0.5, color),
        );
        x += grid_size;
    }

    // Horizontal lines
    let mut y = rect.top();
    while y < rect.bottom() {
        painter.line_segment(
            [egui::pos2(rect.left(), y), egui::pos2(rect.right(), y)],
            egui::Stroke::new(0.5, color),
        );
        y += grid_size;
    }
}

fn draw_connection(
    painter: &egui::Painter,
    graph: &MaterialGraph,
    connection: &MaterialConnection,
    canvas_rect: egui::Rect,
) {
    if let (Some(from_node), Some(to_node)) = (
        graph.nodes.iter().find(|n| n.id == connection.from_node),
        graph.nodes.iter().find(|n| n.id == connection.to_node),
    ) {
        let from_pos = egui::pos2(
            canvas_rect.left() + from_node.position[0] + 120.0, // Right side of output node
            canvas_rect.top() + from_node.position[1] + 20.0,
        );
        let to_pos = egui::pos2(
            canvas_rect.left() + to_node.position[0], // Left side of input node
            canvas_rect.top() + to_node.position[1] + 20.0,
        );

        // Draw bezier curve
        let control_offset = (to_pos.x - from_pos.x).abs() * 0.5;
        let control1 = egui::pos2(from_pos.x + control_offset, from_pos.y);
        let control2 = egui::pos2(to_pos.x - control_offset, to_pos.y);

        draw_bezier_curve(painter, from_pos, control1, control2, to_pos);
    }
}

fn draw_bezier_curve(
    painter: &egui::Painter,
    start: egui::Pos2,
    control1: egui::Pos2,
    control2: egui::Pos2,
    end: egui::Pos2,
) {
    let segments = 20;
    let color = egui::Color32::from_rgb(255, 255, 100);

    let mut points = Vec::new();
    for i in 0..=segments {
        let t = i as f32 / segments as f32;
        let point = bezier_point(start, control1, control2, end, t);
        points.push(point);
    }

    for i in 0..points.len() - 1 {
        painter.line_segment(
            [points[i], points[i + 1]],
            egui::Stroke::new(2.0, color),
        );
    }
}

fn bezier_point(
    p0: egui::Pos2,
    p1: egui::Pos2,
    p2: egui::Pos2,
    p3: egui::Pos2,
    t: f32,
) -> egui::Pos2 {
    let t2 = t * t;
    let t3 = t2 * t;
    let mt = 1.0 - t;
    let mt2 = mt * mt;
    let mt3 = mt2 * mt;

    egui::pos2(
        mt3 * p0.x + 3.0 * mt2 * t * p1.x + 3.0 * mt * t2 * p2.x + t3 * p3.x,
        mt3 * p0.y + 3.0 * mt2 * t * p1.y + 3.0 * mt * t2 * p2.y + t3 * p3.y,
    )
}

fn draw_node(
    painter: &egui::Painter,
    ui: &mut egui::Ui,
    node: &mut MaterialNode,
    canvas_rect: egui::Rect,
    editor_state: &mut MaterialEditorState,
    response: &egui::Response,
    graph: &MaterialGraph,
) {
    let node_pos = egui::pos2(
        canvas_rect.left() + node.position[0],
        canvas_rect.top() + node.position[1],
    );

    // Calculate node size based on content
    let base_width = 140.0; // Keep consistent with interaction functions
    let base_height = 60.0;
    let property_height = match node.node_type {
        MaterialNodeType::ColorConstant => 30.0, // Color picker
        MaterialNodeType::ScalarConstant => 25.0, // Slider
        MaterialNodeType::Vector3Constant => 25.0, // Three sliders
        MaterialNodeType::CheckerPattern => 60.0, // Two color pickers + slider
        MaterialNodeType::ColorMix => 60.0, // Two color pickers + slider
        MaterialNodeType::NoiseTexture => 75.0, // Multiple sliders
        MaterialNodeType::Remap => 100.0, // Many sliders
        _ => 0.0,
    };

    let input_count = node.inputs.len() as f32;
    let output_count = get_output_count_for_node_type(&node.node_type) as f32;
    let socket_height = (input_count.max(output_count) * 20.0).max(20.0);

    let node_height = base_height + property_height + socket_height;
    let node_size = egui::vec2(base_width, node_height);
    let node_rect = egui::Rect::from_min_size(node_pos, node_size);

    // Check if this node is selected
    let is_selected = editor_state.selected_node == Some(node.id);

    // Node background color
    let mut node_color = match node.node_type {
        MaterialNodeType::MaterialOutput => egui::Color32::from_rgb(100, 200, 100),

        // Input nodes - Reddish colors
        MaterialNodeType::ColorConstant => egui::Color32::from_rgb(200, 100, 100),
        MaterialNodeType::ScalarConstant => egui::Color32::from_rgb(100, 100, 200),
        MaterialNodeType::Vector3Constant => egui::Color32::from_rgb(200, 100, 200),
        MaterialNodeType::TextureSample => egui::Color32::from_rgb(200, 150, 100),

        // PBR nodes - Greenish colors
        MaterialNodeType::Fresnel => egui::Color32::from_rgb(100, 200, 150),
        MaterialNodeType::NormalMap => egui::Color32::from_rgb(150, 200, 100),
        MaterialNodeType::DetailNormal => egui::Color32::from_rgb(120, 200, 120),

        // Pattern nodes - Purplish colors
        MaterialNodeType::NoiseTexture => egui::Color32::from_rgb(180, 100, 200),
        MaterialNodeType::CheckerPattern => egui::Color32::from_rgb(200, 100, 180),

        // Utility nodes - Yellowish colors
        MaterialNodeType::ColorMix => egui::Color32::from_rgb(200, 200, 100),
        MaterialNodeType::Clamp => egui::Color32::from_rgb(180, 180, 100),
        MaterialNodeType::Remap => egui::Color32::from_rgb(200, 180, 100),

        // Math nodes - Blueish colors
        MaterialNodeType::Multiply => egui::Color32::from_rgb(100, 150, 200),
        MaterialNodeType::Add => egui::Color32::from_rgb(120, 150, 200),
        MaterialNodeType::Subtract => egui::Color32::from_rgb(140, 150, 200),
        MaterialNodeType::Lerp => egui::Color32::from_rgb(100, 170, 200),
        MaterialNodeType::Power => egui::Color32::from_rgb(160, 150, 200),
        MaterialNodeType::OneMinus => egui::Color32::from_rgb(180, 150, 200),
    };

    // Brighten color if selected
    if is_selected {
        let r = ((node_color.r() as f32 * 1.3).min(255.0)) as u8;
        let g = ((node_color.g() as f32 * 1.3).min(255.0)) as u8;
        let b = ((node_color.b() as f32 * 1.3).min(255.0)) as u8;
        node_color = egui::Color32::from_rgb(r, g, b);
    }

    painter.rect_filled(node_rect, egui::Rounding::same(5.0), node_color);

    // Draw selection border
    let border_color = if is_selected {
        egui::Color32::from_rgb(255, 255, 0) // Yellow border for selected node
    } else {
        egui::Color32::WHITE
    };
    let border_width = if is_selected { 3.0 } else { 1.0 };
    painter.rect_stroke(node_rect, egui::Rounding::same(5.0), egui::Stroke::new(border_width, border_color));

    // Node title
    painter.text(
        egui::pos2(node_pos.x + 5.0, node_pos.y + 5.0),
        egui::Align2::LEFT_TOP,
        &node.name,
        egui::FontId::default(),
        egui::Color32::WHITE,
    );

    // Property editing area - positioned below the title
    let properties_rect = egui::Rect::from_min_size(
        egui::pos2(node_pos.x + 5.0, node_pos.y + 25.0),
        egui::vec2(base_width - 10.0, property_height)
    );

    // Draw property editing widgets
    if property_height > 0.0 {
        let mut property_ui = ui.child_ui(properties_rect, egui::Layout::top_down(egui::Align::LEFT));
        draw_node_properties(&mut property_ui, node);
    }

    // Calculate socket positions - below properties
    let socket_start_y = node_pos.y + base_height + property_height;

    // Draw input sockets (left side)
    let mut input_y = socket_start_y;
    for (input_name, _value) in &node.inputs {
        let socket_pos = egui::pos2(node_pos.x - 5.0, input_y);
        draw_input_socket(painter, ui, socket_pos, input_name, node.id, &node.node_type, editor_state, response, graph);

        // Draw input label
        painter.text(
            egui::pos2(node_pos.x + 8.0, input_y - 6.0),
            egui::Align2::LEFT_TOP,
            input_name,
            egui::FontId::proportional(10.0),
            egui::Color32::WHITE,
        );

        input_y += 20.0;
    }

    // Draw output sockets (right side) - most nodes have one output
    if node.node_type != MaterialNodeType::MaterialOutput {
        let output_names = get_output_names_for_node_type(&node.node_type);
        let mut output_y = socket_start_y;

        for output_name in output_names {
            let socket_pos = egui::pos2(node_pos.x + base_width + 5.0, output_y);
            draw_output_socket(painter, ui, socket_pos, &output_name, node.id, &node.node_type, editor_state, response);

            // Draw output label
            painter.text(
                egui::pos2(node_pos.x + base_width - 8.0, output_y - 6.0),
                egui::Align2::RIGHT_TOP,
                &output_name,
                egui::FontId::proportional(10.0),
                egui::Color32::WHITE,
            );

            output_y += 20.0;
        }
    }
}

fn get_output_count_for_node_type(node_type: &MaterialNodeType) -> usize {
    match node_type {
        MaterialNodeType::MaterialOutput => 0,
        _ => 1, // Most nodes have one output
    }
}

fn get_output_names_for_node_type(node_type: &MaterialNodeType) -> Vec<String> {
    match node_type {
        // Input nodes
        MaterialNodeType::ColorConstant => vec!["Color".to_string()],
        MaterialNodeType::ScalarConstant => vec!["Value".to_string()],
        MaterialNodeType::Vector3Constant => vec!["Vector".to_string()],
        MaterialNodeType::TextureSample => vec!["RGB".to_string(), "Alpha".to_string()],

        // PBR nodes
        MaterialNodeType::Fresnel => vec!["Fresnel".to_string()],
        MaterialNodeType::NormalMap => vec!["Normal".to_string()],
        MaterialNodeType::DetailNormal => vec!["Normal".to_string()],

        // Pattern nodes
        MaterialNodeType::NoiseTexture => vec!["Noise".to_string()],
        MaterialNodeType::CheckerPattern => vec!["Pattern".to_string()],

        // Utility nodes
        MaterialNodeType::ColorMix => vec!["Color".to_string()],
        MaterialNodeType::Clamp => vec!["Result".to_string()],
        MaterialNodeType::Remap => vec!["Result".to_string()],

        // Math nodes
        MaterialNodeType::Multiply | MaterialNodeType::Add | MaterialNodeType::Subtract => vec!["Result".to_string()],
        MaterialNodeType::Lerp => vec!["Result".to_string()],
        MaterialNodeType::Power => vec!["Result".to_string()],
        MaterialNodeType::OneMinus => vec!["Result".to_string()],

        MaterialNodeType::MaterialOutput => vec![], // No outputs
    }
}

fn draw_node_properties(ui: &mut egui::Ui, node: &mut MaterialNode) {
    match node.node_type {
        MaterialNodeType::ColorConstant => {
            if let Some(MaterialValue::Color(color)) = node.inputs.get_mut("Color") {
                ui.horizontal(|ui| {
                    ui.label("Color:");
                    ui.color_edit_button_rgba_premultiplied(color);
                });
            }
        },
        MaterialNodeType::ScalarConstant => {
            if let Some(MaterialValue::Scalar(value)) = node.inputs.get_mut("Value") {
                ui.horizontal(|ui| {
                    ui.label("Value:");
                    ui.add(egui::Slider::new(value, 0.0..=10.0).step_by(0.01));
                });
            }
        },
        MaterialNodeType::Vector3Constant => {
            if let Some(MaterialValue::Vector3(vec)) = node.inputs.get_mut("Vector") {
                ui.horizontal(|ui| {
                    ui.label("X:");
                    ui.add(egui::Slider::new(&mut vec[0], -10.0..=10.0).step_by(0.01));
                });
                ui.horizontal(|ui| {
                    ui.label("Y:");
                    ui.add(egui::Slider::new(&mut vec[1], -10.0..=10.0).step_by(0.01));
                });
                ui.horizontal(|ui| {
                    ui.label("Z:");
                    ui.add(egui::Slider::new(&mut vec[2], -10.0..=10.0).step_by(0.01));
                });
            }
        },
        MaterialNodeType::CheckerPattern => {
            if let Some(MaterialValue::Color(color1)) = node.inputs.get_mut("Color1") {
                ui.horizontal(|ui| {
                    ui.label("Color 1:");
                    ui.color_edit_button_rgba_premultiplied(color1);
                });
            }
            if let Some(MaterialValue::Color(color2)) = node.inputs.get_mut("Color2") {
                ui.horizontal(|ui| {
                    ui.label("Color 2:");
                    ui.color_edit_button_rgba_premultiplied(color2);
                });
            }
            if let Some(MaterialValue::Scalar(scale)) = node.inputs.get_mut("Scale") {
                ui.horizontal(|ui| {
                    ui.label("Scale:");
                    ui.add(egui::Slider::new(scale, 1.0..=50.0).step_by(0.1));
                });
            }
        },
        MaterialNodeType::ColorMix => {
            if let Some(MaterialValue::Color(color_a)) = node.inputs.get_mut("Color A") {
                ui.horizontal(|ui| {
                    ui.label("Color A:");
                    ui.color_edit_button_rgba_premultiplied(color_a);
                });
            }
            if let Some(MaterialValue::Color(color_b)) = node.inputs.get_mut("Color B") {
                ui.horizontal(|ui| {
                    ui.label("Color B:");
                    ui.color_edit_button_rgba_premultiplied(color_b);
                });
            }
            if let Some(MaterialValue::Scalar(factor)) = node.inputs.get_mut("Factor") {
                ui.horizontal(|ui| {
                    ui.label("Factor:");
                    ui.add(egui::Slider::new(factor, 0.0..=1.0).step_by(0.01));
                });
            }
        },
        MaterialNodeType::NoiseTexture => {
            if let Some(MaterialValue::Scalar(scale)) = node.inputs.get_mut("Scale") {
                ui.horizontal(|ui| {
                    ui.label("Scale:");
                    ui.add(egui::Slider::new(scale, 0.1..=50.0).step_by(0.1));
                });
            }
            if let Some(MaterialValue::Scalar(detail)) = node.inputs.get_mut("Detail") {
                ui.horizontal(|ui| {
                    ui.label("Detail:");
                    ui.add(egui::Slider::new(detail, 0.0..=10.0).step_by(0.1));
                });
            }
            if let Some(MaterialValue::Scalar(distortion)) = node.inputs.get_mut("Distortion") {
                ui.horizontal(|ui| {
                    ui.label("Distortion:");
                    ui.add(egui::Slider::new(distortion, 0.0..=5.0).step_by(0.01));
                });
            }
        },
        MaterialNodeType::Fresnel => {
            if let Some(MaterialValue::Scalar(ior)) = node.inputs.get_mut("IOR") {
                ui.horizontal(|ui| {
                    ui.label("IOR:");
                    ui.add(egui::Slider::new(ior, 1.0..=3.0).step_by(0.01));
                });
            }
        },
        MaterialNodeType::NormalMap => {
            if let Some(MaterialValue::Scalar(strength)) = node.inputs.get_mut("Strength") {
                ui.horizontal(|ui| {
                    ui.label("Strength:");
                    ui.add(egui::Slider::new(strength, 0.0..=2.0).step_by(0.01));
                });
            }
        },
        MaterialNodeType::Clamp => {
            if let Some(MaterialValue::Scalar(min_val)) = node.inputs.get_mut("Min") {
                ui.horizontal(|ui| {
                    ui.label("Min:");
                    ui.add(egui::Slider::new(min_val, -10.0..=10.0).step_by(0.01));
                });
            }
            if let Some(MaterialValue::Scalar(max_val)) = node.inputs.get_mut("Max") {
                ui.horizontal(|ui| {
                    ui.label("Max:");
                    ui.add(egui::Slider::new(max_val, -10.0..=10.0).step_by(0.01));
                });
            }
        },
        MaterialNodeType::Remap => {
            if let Some(MaterialValue::Scalar(from_min)) = node.inputs.get_mut("From Min") {
                ui.horizontal(|ui| {
                    ui.label("From Min:");
                    ui.add(egui::Slider::new(from_min, -10.0..=10.0).step_by(0.01));
                });
            }
            if let Some(MaterialValue::Scalar(from_max)) = node.inputs.get_mut("From Max") {
                ui.horizontal(|ui| {
                    ui.label("From Max:");
                    ui.add(egui::Slider::new(from_max, -10.0..=10.0).step_by(0.01));
                });
            }
            if let Some(MaterialValue::Scalar(to_min)) = node.inputs.get_mut("To Min") {
                ui.horizontal(|ui| {
                    ui.label("To Min:");
                    ui.add(egui::Slider::new(to_min, -10.0..=10.0).step_by(0.01));
                });
            }
            if let Some(MaterialValue::Scalar(to_max)) = node.inputs.get_mut("To Max") {
                ui.horizontal(|ui| {
                    ui.label("To Max:");
                    ui.add(egui::Slider::new(to_max, -10.0..=10.0).step_by(0.01));
                });
            }
        },
        MaterialNodeType::Power => {
            if let Some(MaterialValue::Scalar(exponent)) = node.inputs.get_mut("Exponent") {
                ui.horizontal(|ui| {
                    ui.label("Exponent:");
                    ui.add(egui::Slider::new(exponent, 0.1..=10.0).step_by(0.01));
                });
            }
        },
        _ => {
            // For other node types, show simple value editors for their inputs
            for (input_name, input_value) in &mut node.inputs {
                match input_value {
                    MaterialValue::Scalar(val) => {
                        ui.horizontal(|ui| {
                            ui.label(format!("{}:", input_name));
                            ui.add(egui::Slider::new(val, 0.0..=10.0).step_by(0.01));
                        });
                    },
                    MaterialValue::Color(color) => {
                        ui.horizontal(|ui| {
                            ui.label(format!("{}:", input_name));
                            ui.color_edit_button_rgba_premultiplied(color);
                        });
                    },
                    _ => {}
                }
            }
        }
    }
}

fn draw_input_socket(
    painter: &egui::Painter,
    ui: &mut egui::Ui,
    socket_pos: egui::Pos2,
    input_name: &str,
    node_id: u32,
    node_type: &MaterialNodeType,
    editor_state: &mut MaterialEditorState,
    response: &egui::Response,
    graph: &MaterialGraph,
) {
    let socket_radius = 6.0;
    let socket_rect = egui::Rect::from_center_size(socket_pos, egui::vec2(socket_radius * 2.0, socket_radius * 2.0));

    // Socket color - based on socket type
    let socket_type = get_input_socket_type(node_type, input_name).unwrap_or(SocketType::Scalar);
    let mut socket_color = match socket_type {
        SocketType::Color => egui::Color32::from_rgb(255, 100, 100), // Red for colors
        SocketType::Scalar => egui::Color32::from_rgb(100, 255, 100), // Green for scalars
        SocketType::Vector3 => egui::Color32::from_rgb(100, 100, 255), // Blue for vectors
        SocketType::Texture => egui::Color32::from_rgb(255, 255, 100), // Yellow for textures
    };

    // Check if hovered and brighten if so
    if let Some(pointer_pos) = response.interact_pointer_pos() {
        if socket_rect.contains(pointer_pos) {
            let r = ((socket_color.r() as f32 * 1.5).min(255.0)) as u8;
            let g = ((socket_color.g() as f32 * 1.5).min(255.0)) as u8;
            let b = ((socket_color.b() as f32 * 1.5).min(255.0)) as u8;
            socket_color = egui::Color32::from_rgb(r, g, b);
        }
    }

    painter.circle_filled(socket_pos, socket_radius, socket_color);

    // Check if we're creating a connection to highlight potential targets
    let border_color = if let Some((from_node_id, from_output)) = &editor_state.creating_connection {
        // Find the from node to check connection validity
        let is_valid = graph.nodes.iter().find(|n| n.id == *from_node_id)
            .and_then(|from_node| graph.nodes.iter().find(|n| n.id == node_id)
                .map(|to_node| is_connection_valid(&from_node.node_type, from_output, &to_node.node_type, input_name)))
            .unwrap_or(false);

        if is_valid {
            egui::Color32::from_rgb(0, 255, 0) // Green border for valid connection target
        } else {
            egui::Color32::from_rgb(255, 0, 0) // Red border for invalid connection target
        }
    } else {
        egui::Color32::WHITE
    };
    painter.circle_stroke(socket_pos, socket_radius, egui::Stroke::new(1.0, border_color));
}

fn draw_output_socket(
    painter: &egui::Painter,
    ui: &mut egui::Ui,
    socket_pos: egui::Pos2,
    output_name: &str,
    node_id: u32,
    node_type: &MaterialNodeType,
    editor_state: &mut MaterialEditorState,
    response: &egui::Response,
) {
    let socket_radius = 6.0;
    let socket_rect = egui::Rect::from_center_size(socket_pos, egui::vec2(socket_radius * 2.0, socket_radius * 2.0));

    // Socket color - based on socket type
    let socket_type = get_output_socket_type(node_type, output_name).unwrap_or(SocketType::Scalar);
    let mut socket_color = match socket_type {
        SocketType::Color => egui::Color32::from_rgb(255, 100, 100), // Red for colors
        SocketType::Scalar => egui::Color32::from_rgb(100, 255, 100), // Green for scalars
        SocketType::Vector3 => egui::Color32::from_rgb(100, 100, 255), // Blue for vectors
        SocketType::Texture => egui::Color32::from_rgb(255, 255, 100), // Yellow for textures
    };

    // Check if hovered and brighten if so
    if let Some(pointer_pos) = response.interact_pointer_pos() {
        if socket_rect.contains(pointer_pos) {
            let r = ((socket_color.r() as f32 * 1.5).min(255.0)) as u8;
            let g = ((socket_color.g() as f32 * 1.5).min(255.0)) as u8;
            let b = ((socket_color.b() as f32 * 1.5).min(255.0)) as u8;
            socket_color = egui::Color32::from_rgb(r, g, b);
        }
    }

    painter.circle_filled(socket_pos, socket_radius, socket_color);

    // Highlight if this socket is being used for connection creation
    let border_color = if let Some((from_node_id, from_output)) = &editor_state.creating_connection {
        if *from_node_id == node_id && from_output == output_name {
            egui::Color32::from_rgb(255, 255, 0) // Yellow border for active connection source
        } else {
            egui::Color32::WHITE
        }
    } else {
        egui::Color32::WHITE
    };
    painter.circle_stroke(socket_pos, socket_radius, egui::Stroke::new(2.0, border_color));
}

fn handle_node_interactions(
    graph: &mut MaterialGraph,
    editor_state: &mut MaterialEditorState,
    response: &egui::Response,
    canvas_rect: egui::Rect,
) {
    // Handle socket interactions for each node - need to collect node data first to avoid borrowing issues
    let node_count = graph.nodes.len();
    for i in 0..node_count {
        let node_id = graph.nodes[i].id;
        let node_position = graph.nodes[i].position;
        let node_type = graph.nodes[i].node_type.clone();
        let node_inputs: Vec<(String, MaterialValue)> = graph.nodes[i].inputs.iter().map(|(k, v)| (k.clone(), v.clone())).collect();

        // Check input socket interactions
        let node_pos = egui::pos2(
            canvas_rect.left() + node_position[0],
            canvas_rect.top() + node_position[1],
        );

        // Calculate socket start position using same logic as draw_node
        let base_height = 60.0;
        let property_height = match node_type {
            MaterialNodeType::ColorConstant => 30.0,
            MaterialNodeType::ScalarConstant => 25.0,
            MaterialNodeType::Vector3Constant => 25.0,
            MaterialNodeType::CheckerPattern => 60.0,
            MaterialNodeType::ColorMix => 60.0,
            MaterialNodeType::NoiseTexture => 75.0,
            MaterialNodeType::Remap => 100.0,
            _ => 0.0,
        };
        let socket_start_y = node_pos.y + base_height + property_height;

        let mut input_y = socket_start_y;
        for (input_name, _value) in &node_inputs {
            let socket_pos = egui::pos2(node_pos.x - 5.0, input_y);
            let socket_radius = 6.0;
            let socket_rect = egui::Rect::from_center_size(socket_pos, egui::vec2(socket_radius * 2.0, socket_radius * 2.0));

            if let Some(pointer_pos) = response.interact_pointer_pos() {
                if socket_rect.contains(pointer_pos) {
                    // Handle connection ending on this input when drag ends
                    if response.drag_released() {
                        if let Some((from_node_id, from_output)) = &editor_state.creating_connection.clone() {
                            // Find the from and to nodes to validate connection
                            let from_node = graph.nodes.iter().find(|n| n.id == *from_node_id);
                            let to_node = graph.nodes.iter().find(|n| n.id == node_id);

                            if let (Some(from_node), Some(to_node)) = (from_node, to_node) {
                                // Validate connection types
                                if is_connection_valid(&from_node.node_type, from_output, &to_node.node_type, input_name) {
                                    // Create valid connection
                                    let connection = MaterialConnection {
                                        from_node: *from_node_id,
                                        from_output: from_output.clone(),
                                        to_node: node_id,
                                        to_input: input_name.clone(),
                                    };

                                    // Remove any existing connection to this input
                                    graph.connections.retain(|c| !(c.to_node == node_id && c.to_input == *input_name));

                                    // Add the new connection
                                    graph.connections.push(connection);

                                    println!("Created valid connection from node {} output '{}' to node {} input '{}'",
                                        from_node_id, from_output, node_id, input_name);
                                } else {
                                    println!("Invalid connection: Cannot connect {:?} output '{}' to {:?} input '{}'",
                                        from_node.node_type, from_output, to_node.node_type, input_name);
                                }
                            }

                            editor_state.creating_connection = None;
                        }
                    }

                    // Handle disconnection on right-click
                    if response.secondary_clicked() {
                        if socket_rect.contains(response.interact_pointer_pos().unwrap_or_default()) {
                            // Remove any connection to this input
                            let connections_removed = graph.connections.len();
                            graph.connections.retain(|c| !(c.to_node == node_id && c.to_input == *input_name));
                            let new_count = graph.connections.len();

                            if connections_removed > new_count {
                                println!("Disconnected input '{}' on node {}", input_name, node_id);
                            }
                        }
                    }
                }
            }

            input_y += 20.0;
        }

        // Check output socket interactions (if not MaterialOutput)
        if node_type != MaterialNodeType::MaterialOutput {
            let output_names = get_output_names_for_node_type(&node_type);
            let mut output_y = socket_start_y;
            let node_width = 140.0;

            for output_name in output_names {
                let socket_pos = egui::pos2(node_pos.x + node_width + 5.0, output_y);
                let socket_radius = 6.0;
                let socket_rect = egui::Rect::from_center_size(socket_pos, egui::vec2(socket_radius * 2.0, socket_radius * 2.0));

                if let Some(pointer_pos) = response.interact_pointer_pos() {
                    if socket_rect.contains(pointer_pos) {
                        if response.drag_started() {
                            // Start creating a connection from this output when drag starts
                            editor_state.creating_connection = Some((node_id, output_name.clone()));
                            println!("Starting connection from node {} output '{}'", node_id, output_name);
                        }
                    }
                }

                output_y += 20.0;
            }
        }
    }

    // Handle node dragging
    if response.dragged() {
        if let Some(dragging_node_id) = editor_state.dragging_node {
            if let Some(node) = graph.nodes.iter_mut().find(|n| n.id == dragging_node_id) {
                node.position[0] += response.drag_delta().x;
                node.position[1] += response.drag_delta().y;
            }
        }
    }

    // Handle immediate node selection on mouse press
    if response.clicked() {
        if let Some(pointer_pos) = response.interact_pointer_pos() {
            // Convert pointer position to canvas coordinates
            let canvas_pointer_pos = egui::pos2(
                pointer_pos.x - canvas_rect.left(),
                pointer_pos.y - canvas_rect.top(),
            );

            // Find which node was clicked for selection
            let mut node_clicked = false;
            for node in &graph.nodes {
                let node_pos = egui::pos2(node.position[0], node.position[1]);

                // Calculate node size using same logic as draw_node
                let base_width = 140.0;
                let base_height = 60.0;
                let property_height = match node.node_type {
                    MaterialNodeType::ColorConstant => 30.0,
                    MaterialNodeType::ScalarConstant => 25.0,
                    MaterialNodeType::Vector3Constant => 25.0,
                    MaterialNodeType::CheckerPattern => 60.0,
                    MaterialNodeType::ColorMix => 60.0,
                    MaterialNodeType::NoiseTexture => 75.0,
                    MaterialNodeType::Remap => 100.0,
                    _ => 0.0,
                };

                let input_count = node.inputs.len() as f32;
                let output_count = get_output_count_for_node_type(&node.node_type) as f32;
                let socket_height = (input_count.max(output_count) * 20.0).max(20.0);

                let node_height = base_height + property_height + socket_height;
                let node_size = egui::vec2(base_width, node_height);
                let node_rect = egui::Rect::from_min_size(node_pos, node_size);

                if node_rect.contains(canvas_pointer_pos) {
                    editor_state.selected_node = Some(node.id);
                    node_clicked = true;
                    println!("Selected node {}", node.id);
                    break;
                }
            }

            // If clicked on empty space, clear selection
            if !node_clicked {
                editor_state.selected_node = None;
                editor_state.creating_connection = None; // Also cancel any connection creation
                println!("Cleared node selection");
            }
        }
    }

    // Handle clicking to start dragging (only if not already creating a connection)
    if response.drag_started() && editor_state.creating_connection.is_none() {
        if let Some(pointer_pos) = response.interact_pointer_pos() {
            // Convert pointer position to canvas coordinates
            let canvas_pointer_pos = egui::pos2(
                pointer_pos.x - canvas_rect.left(),
                pointer_pos.y - canvas_rect.top(),
            );

            // Check if we're not clicking on a socket before allowing node drag
            let mut clicking_socket = false;

            // Check all nodes for socket clicks
            for node in &graph.nodes {
                let node_pos = egui::pos2(node.position[0], node.position[1]);
                let node_width = 140.0;

                // Calculate socket start position using same logic as draw_node
                let base_height = 60.0;
                let property_height = match node.node_type {
                    MaterialNodeType::ColorConstant => 30.0,
                    MaterialNodeType::ScalarConstant => 25.0,
                    MaterialNodeType::Vector3Constant => 25.0,
                    MaterialNodeType::CheckerPattern => 60.0,
                    MaterialNodeType::ColorMix => 60.0,
                    MaterialNodeType::NoiseTexture => 75.0,
                    MaterialNodeType::Remap => 100.0,
                    _ => 0.0,
                };
                let socket_start_y = node_pos.y + base_height + property_height;

                // Check input sockets
                let mut input_y = socket_start_y;
                for _ in &node.inputs {
                    let socket_pos = egui::pos2(node_pos.x - 5.0, input_y);
                    let socket_radius = 6.0;
                    let socket_rect = egui::Rect::from_center_size(socket_pos, egui::vec2(socket_radius * 2.0, socket_radius * 2.0));
                    if socket_rect.contains(canvas_pointer_pos) {
                        clicking_socket = true;
                        break;
                    }
                    input_y += 20.0;
                }

                // Check output sockets (if not MaterialOutput)
                if node.node_type != MaterialNodeType::MaterialOutput {
                    let output_names = get_output_names_for_node_type(&node.node_type);
                    let mut output_y = socket_start_y;
                    for _ in output_names {
                        let socket_pos = egui::pos2(node_pos.x + node_width + 5.0, output_y);
                        let socket_radius = 6.0;
                        let socket_rect = egui::Rect::from_center_size(socket_pos, egui::vec2(socket_radius * 2.0, socket_radius * 2.0));
                        if socket_rect.contains(canvas_pointer_pos) {
                            clicking_socket = true;
                            break;
                        }
                        output_y += 20.0;
                    }
                }

                if clicking_socket {
                    break;
                }
            }

            // Only start node dragging if not clicking on a socket
            if !clicking_socket {
                // Find which node was clicked
                for node in &graph.nodes {
                    let node_pos = egui::pos2(node.position[0], node.position[1]);

                    // Calculate node size using same logic as draw_node
                    let base_width = 140.0;
                    let base_height = 60.0;
                    let property_height = match node.node_type {
                        MaterialNodeType::ColorConstant => 30.0,
                        MaterialNodeType::ScalarConstant => 25.0,
                        MaterialNodeType::Vector3Constant => 25.0,
                        MaterialNodeType::CheckerPattern => 60.0,
                        MaterialNodeType::ColorMix => 60.0,
                        MaterialNodeType::NoiseTexture => 75.0,
                        MaterialNodeType::Remap => 100.0,
                        _ => 0.0,
                    };

                    let input_count = node.inputs.len() as f32;
                    let output_count = get_output_count_for_node_type(&node.node_type) as f32;
                    let socket_height = (input_count.max(output_count) * 20.0).max(20.0);

                    let node_height = base_height + property_height + socket_height;
                    let node_size = egui::vec2(base_width, node_height);
                    let node_rect = egui::Rect::from_min_size(node_pos, node_size);

                    if node_rect.contains(canvas_pointer_pos) {
                        editor_state.dragging_node = Some(node.id);
                        break;
                    }
                }
            }
        }
    }

    // Stop dragging
    if response.drag_released() {
        editor_state.dragging_node = None;
    }

    // Cancel connection creation on right click or escape
    if response.secondary_clicked() {
        editor_state.creating_connection = None;
    }
}

fn get_output_socket_pos(node: &MaterialNode, output_name: &str, canvas_rect: egui::Rect) -> egui::Pos2 {
    let node_pos = egui::pos2(
        canvas_rect.left() + node.position[0],
        canvas_rect.top() + node.position[1],
    );

    let node_width = 140.0;
    let output_names = get_output_names_for_node_type(&node.node_type);

    for (i, name) in output_names.iter().enumerate() {
        if name == output_name {
            let output_y = node_pos.y + 25.0 + (i as f32 * 20.0);
            return egui::pos2(node_pos.x + node_width + 5.0, output_y);
        }
    }

    node_pos // fallback
}

fn draw_preview_connection(painter: &egui::Painter, from_pos: egui::Pos2, to_pos: egui::Pos2) {
    let control_offset = (to_pos.x - from_pos.x).abs() * 0.5;
    let control1 = egui::pos2(from_pos.x + control_offset, from_pos.y);
    let control2 = egui::pos2(to_pos.x - control_offset, to_pos.y);

    // Draw a dashed preview line
    let segments = 10;
    let color = egui::Color32::from_rgba_premultiplied(255, 255, 100, 150);

    for i in 0..segments {
        let t1 = i as f32 / segments as f32;
        let t2 = (i + 1) as f32 / segments as f32;
        let p1 = bezier_point(from_pos, control1, control2, to_pos, t1);
        let p2 = bezier_point(from_pos, control1, control2, to_pos, t2);

        painter.line_segment([p1, p2], egui::Stroke::new(2.0, color));
    }
}

fn compile_material_from_graph(material: &mut SproutMaterial) {
    // Find the Material Output node
    if let Some(output_node) = material.material_graph.nodes.iter()
        .find(|n| n.node_type == MaterialNodeType::MaterialOutput) {

        // Process Base Color
        if let Some(base_color_value) = evaluate_node_input(&material.material_graph, output_node.id, "Base Color") {
            if let MaterialValue::Color(color) = base_color_value {
                material.albedo_color = color;
                println!("Updated material base color to: {:?}", color);
            }
        } else if let Some(base_color) = output_node.inputs.get("Base Color") {
            if let MaterialValue::Color(color) = base_color {
                material.albedo_color = *color;
            }
        }

        // Process Metallic
        if let Some(metallic_value) = evaluate_node_input(&material.material_graph, output_node.id, "Metallic") {
            if let MaterialValue::Scalar(value) = metallic_value {
                material.metallic = value;
                println!("Updated material metallic to: {}", value);
            }
        } else if let Some(metallic) = output_node.inputs.get("Metallic") {
            if let MaterialValue::Scalar(value) = metallic {
                material.metallic = *value;
            }
        }

        // Process Roughness
        if let Some(roughness_value) = evaluate_node_input(&material.material_graph, output_node.id, "Roughness") {
            if let MaterialValue::Scalar(value) = roughness_value {
                material.roughness = value;
                println!("Updated material roughness to: {}", value);
            }
        } else if let Some(roughness) = output_node.inputs.get("Roughness") {
            if let MaterialValue::Scalar(value) = roughness {
                material.roughness = *value;
            }
        }

        // Process Emission
        if let Some(emission_value) = evaluate_node_input(&material.material_graph, output_node.id, "Emission") {
            if let MaterialValue::Color(color) = emission_value {
                material.emission = [color[0], color[1], color[2]];
                material.emission_strength = color[3]; // Use alpha as strength
                println!("Updated material emission to: {:?}", color);
            }
        } else if let Some(emission) = output_node.inputs.get("Emission") {
            if let MaterialValue::Color(color) = emission {
                material.emission = [color[0], color[1], color[2]];
                material.emission_strength = color[3]; // Use alpha as strength
            }
        }
    }
}

// Evaluate a node input by following connections
fn evaluate_node_input(graph: &MaterialGraph, node_id: u32, input_name: &str) -> Option<MaterialValue> {
    // Check if there's a connection to this input
    if let Some(connection) = graph.connections.iter().find(|c| c.to_node == node_id && c.to_input == input_name) {
        // Find the source node
        if let Some(source_node) = graph.nodes.iter().find(|n| n.id == connection.from_node) {
            // Evaluate the source node and return its output
            return evaluate_node_output(graph, source_node, &connection.from_output);
        }
    }
    None
}

// Evaluate a node's output value
fn evaluate_node_output(graph: &MaterialGraph, node: &MaterialNode, output_name: &str) -> Option<MaterialValue> {
    match node.node_type {
        // Input nodes
        MaterialNodeType::ColorConstant => {
            if output_name == "Color" {
                if let Some(MaterialValue::Color(color)) = node.inputs.get("Color") {
                    return Some(MaterialValue::Color(*color));
                }
            }
        }
        MaterialNodeType::ScalarConstant => {
            if output_name == "Value" {
                if let Some(MaterialValue::Scalar(value)) = node.inputs.get("Value") {
                    return Some(MaterialValue::Scalar(*value));
                }
            }
        }
        MaterialNodeType::Vector3Constant => {
            if output_name == "Vector" {
                if let Some(MaterialValue::Vector3(vec)) = node.inputs.get("Vector") {
                    return Some(MaterialValue::Vector3(*vec));
                }
            }
        }

        // Pattern nodes
        MaterialNodeType::CheckerPattern => {
            if output_name == "Pattern" {
                // Simple alternating pattern - in real implementation this would sample based on UV
                let color1 = node.inputs.get("Color1").cloned().unwrap_or(MaterialValue::Color([1.0, 1.0, 1.0, 1.0]));
                let color2 = node.inputs.get("Color2").cloned().unwrap_or(MaterialValue::Color([0.0, 0.0, 0.0, 1.0]));
                // For now, just return color1 (in real implementation would compute based on UV coords)
                return Some(color1);
            }
        }
        MaterialNodeType::NoiseTexture => {
            if output_name == "Noise" {
                // Simple noise approximation
                let scale = if let Some(MaterialValue::Scalar(s)) = node.inputs.get("Scale") { *s } else { 1.0 };
                let noise_value = (scale * 0.5).sin() * 0.5 + 0.5; // Simple sine-based noise
                return Some(MaterialValue::Scalar(noise_value));
            }
        }

        // Utility nodes
        MaterialNodeType::ColorMix => {
            if output_name == "Color" {
                let color_a = evaluate_node_input(graph, node.id, "Color A").or_else(|| node.inputs.get("Color A").cloned());
                let color_b = evaluate_node_input(graph, node.id, "Color B").or_else(|| node.inputs.get("Color B").cloned());
                let factor = evaluate_node_input(graph, node.id, "Factor").or_else(|| node.inputs.get("Factor").cloned());

                if let (Some(MaterialValue::Color(a)), Some(MaterialValue::Color(b)), Some(MaterialValue::Scalar(f))) = (color_a, color_b, factor) {
                    let f = f.clamp(0.0, 1.0);
                    return Some(MaterialValue::Color([
                        a[0] * (1.0 - f) + b[0] * f,
                        a[1] * (1.0 - f) + b[1] * f,
                        a[2] * (1.0 - f) + b[2] * f,
                        a[3] * (1.0 - f) + b[3] * f,
                    ]));
                }
            }
        }
        MaterialNodeType::Clamp => {
            if output_name == "Result" {
                let value = evaluate_node_input(graph, node.id, "Value").or_else(|| node.inputs.get("Value").cloned());
                let min_val = if let Some(MaterialValue::Scalar(m)) = node.inputs.get("Min") { *m } else { 0.0 };
                let max_val = if let Some(MaterialValue::Scalar(m)) = node.inputs.get("Max") { *m } else { 1.0 };

                if let Some(MaterialValue::Scalar(v)) = value {
                    return Some(MaterialValue::Scalar(v.clamp(min_val, max_val)));
                }
            }
        }
        MaterialNodeType::Remap => {
            if output_name == "Result" {
                let value = evaluate_node_input(graph, node.id, "Value").or_else(|| node.inputs.get("Value").cloned());
                let from_min = if let Some(MaterialValue::Scalar(m)) = node.inputs.get("From Min") { *m } else { 0.0 };
                let from_max = if let Some(MaterialValue::Scalar(m)) = node.inputs.get("From Max") { *m } else { 1.0 };
                let to_min = if let Some(MaterialValue::Scalar(m)) = node.inputs.get("To Min") { *m } else { 0.0 };
                let to_max = if let Some(MaterialValue::Scalar(m)) = node.inputs.get("To Max") { *m } else { 1.0 };

                if let Some(MaterialValue::Scalar(v)) = value {
                    let normalized = (v - from_min) / (from_max - from_min);
                    let remapped = to_min + normalized * (to_max - to_min);
                    return Some(MaterialValue::Scalar(remapped));
                }
            }
        }

        // PBR nodes
        MaterialNodeType::Fresnel => {
            if output_name == "Fresnel" {
                let ior = if let Some(MaterialValue::Scalar(i)) = node.inputs.get("IOR") { *i } else { 1.45 };
                // Simple Fresnel approximation (in real implementation would use view direction and normal)
                let fresnel = ((ior - 1.0) / (ior + 1.0)).powi(2);
                return Some(MaterialValue::Scalar(fresnel));
            }
        }
        MaterialNodeType::NormalMap | MaterialNodeType::DetailNormal => {
            if output_name == "Normal" {
                // Return default normal
                return Some(MaterialValue::Vector3([0.0, 0.0, 1.0]));
            }
        }

        // Math nodes
        MaterialNodeType::Multiply => {
            if output_name == "Result" {
                let a = evaluate_node_input(graph, node.id, "A").or_else(|| node.inputs.get("A").cloned());
                let b = evaluate_node_input(graph, node.id, "B").or_else(|| node.inputs.get("B").cloned());

                if let (Some(a_val), Some(b_val)) = (a, b) {
                    match (a_val, b_val) {
                        (MaterialValue::Scalar(a), MaterialValue::Scalar(b)) => {
                            return Some(MaterialValue::Scalar(a * b));
                        }
                        (MaterialValue::Color(a), MaterialValue::Color(b)) => {
                            return Some(MaterialValue::Color([
                                a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]
                            ]));
                        }
                        (MaterialValue::Color(color), MaterialValue::Scalar(scalar)) |
                        (MaterialValue::Scalar(scalar), MaterialValue::Color(color)) => {
                            return Some(MaterialValue::Color([
                                color[0] * scalar, color[1] * scalar, color[2] * scalar, color[3]
                            ]));
                        }
                        _ => {}
                    }
                }
            }
        }
        MaterialNodeType::Add => {
            if output_name == "Result" {
                let a = evaluate_node_input(graph, node.id, "A").or_else(|| node.inputs.get("A").cloned());
                let b = evaluate_node_input(graph, node.id, "B").or_else(|| node.inputs.get("B").cloned());

                if let (Some(a_val), Some(b_val)) = (a, b) {
                    match (a_val, b_val) {
                        (MaterialValue::Scalar(a), MaterialValue::Scalar(b)) => {
                            return Some(MaterialValue::Scalar(a + b));
                        }
                        (MaterialValue::Color(a), MaterialValue::Color(b)) => {
                            return Some(MaterialValue::Color([
                                a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]
                            ]));
                        }
                        _ => {}
                    }
                }
            }
        }
        MaterialNodeType::Subtract => {
            if output_name == "Result" {
                let a = evaluate_node_input(graph, node.id, "A").or_else(|| node.inputs.get("A").cloned());
                let b = evaluate_node_input(graph, node.id, "B").or_else(|| node.inputs.get("B").cloned());

                if let (Some(a_val), Some(b_val)) = (a, b) {
                    match (a_val, b_val) {
                        (MaterialValue::Scalar(a), MaterialValue::Scalar(b)) => {
                            return Some(MaterialValue::Scalar(a - b));
                        }
                        (MaterialValue::Color(a), MaterialValue::Color(b)) => {
                            return Some(MaterialValue::Color([
                                a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]
                            ]));
                        }
                        _ => {}
                    }
                }
            }
        }
        MaterialNodeType::Lerp => {
            if output_name == "Result" {
                let a = evaluate_node_input(graph, node.id, "A").or_else(|| node.inputs.get("A").cloned());
                let b = evaluate_node_input(graph, node.id, "B").or_else(|| node.inputs.get("B").cloned());
                let alpha = evaluate_node_input(graph, node.id, "Alpha").or_else(|| node.inputs.get("Alpha").cloned());

                if let (Some(a_val), Some(b_val), Some(MaterialValue::Scalar(alpha_val))) = (a, b, alpha) {
                    let alpha_clamped = alpha_val.clamp(0.0, 1.0);
                    match (a_val, b_val) {
                        (MaterialValue::Color(a), MaterialValue::Color(b)) => {
                            return Some(MaterialValue::Color([
                                a[0] * (1.0 - alpha_clamped) + b[0] * alpha_clamped,
                                a[1] * (1.0 - alpha_clamped) + b[1] * alpha_clamped,
                                a[2] * (1.0 - alpha_clamped) + b[2] * alpha_clamped,
                                a[3] * (1.0 - alpha_clamped) + b[3] * alpha_clamped,
                            ]));
                        }
                        (MaterialValue::Scalar(a), MaterialValue::Scalar(b)) => {
                            return Some(MaterialValue::Scalar(a * (1.0 - alpha_clamped) + b * alpha_clamped));
                        }
                        _ => {}
                    }
                }
            }
        }
        MaterialNodeType::Power => {
            if output_name == "Result" {
                let base = evaluate_node_input(graph, node.id, "Base").or_else(|| node.inputs.get("Base").cloned());
                let exponent = if let Some(MaterialValue::Scalar(e)) = node.inputs.get("Exponent") { *e } else { 2.0 };

                if let Some(MaterialValue::Scalar(base_val)) = base {
                    return Some(MaterialValue::Scalar(base_val.powf(exponent)));
                }
            }
        }
        MaterialNodeType::OneMinus => {
            if output_name == "Result" {
                let input = evaluate_node_input(graph, node.id, "Input").or_else(|| node.inputs.get("Input").cloned());

                if let Some(MaterialValue::Scalar(val)) = input {
                    return Some(MaterialValue::Scalar(1.0 - val));
                }
            }
        }

        // Texture sampling (placeholder)
        MaterialNodeType::TextureSample => {
            match output_name {
                "RGB" => return Some(MaterialValue::Color([0.5, 0.5, 0.5, 1.0])), // Placeholder gray
                "Alpha" => return Some(MaterialValue::Scalar(1.0)),
                _ => {}
            }
        }

        MaterialNodeType::MaterialOutput => {} // Output node has no outputs
    }
    None
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
            .init_resource::<MaterialEditorState>()
            .add_system(material_editor_ui);
    }
}
