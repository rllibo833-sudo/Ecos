extends Node3D

const PLAYER_SCRIPT = preload("res://scripts/player.gd")

func _ready() -> void:
	_build_environment()
	_build_ground()
	_build_landmarks()
	_spawn_player()
	_build_hud()

func _build_environment() -> void:
	var world_environment := WorldEnvironment.new()
	var environment := Environment.new()
	environment.background_mode = Environment.BG_SKY
	var sky := Sky.new()
	var sky_material := ProceduralSkyMaterial.new()
	sky_material.sky_top_color = Color(0.10, 0.30, 0.55)
	sky_material.sky_horizon_color = Color(0.65, 0.78, 0.88)
	sky_material.ground_bottom_color = Color(0.08, 0.10, 0.08)
	sky_material.ground_horizon_color = Color(0.42, 0.48, 0.38)
	sky.sky_material = sky_material
	environment.sky = sky
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	environment.ambient_light_color = Color(0.68, 0.75, 0.84)
	environment.ambient_light_energy = 0.65
	world_environment.environment = environment
	add_child(world_environment)

	var sun := DirectionalLight3D.new()
	sun.rotation_degrees = Vector3(-48.0, -32.0, 0.0)
	sun.light_energy = 1.2
	sun.shadow_enabled = true
	add_child(sun)

func _build_ground() -> void:
	var ground := StaticBody3D.new()
	ground.name = "Ground"
	var mesh_instance := MeshInstance3D.new()
	var plane := PlaneMesh.new()
	plane.size = Vector2(220.0, 220.0)
	mesh_instance.mesh = plane
	var material := StandardMaterial3D.new()
	material.albedo_color = Color(0.22, 0.34, 0.20)
	material.roughness = 1.0
	mesh_instance.material_override = material
	ground.add_child(mesh_instance)
	var collision := CollisionShape3D.new()
	var shape := BoxShape3D.new()
	shape.size = Vector3(220.0, 0.2, 220.0)
	collision.shape = shape
	collision.position.y = -0.1
	ground.add_child(collision)
	add_child(ground)

func _build_landmarks() -> void:
	# Blockout only: no exact geography or distances are asserted as canon.
	_add_building(Vector3(0.0, 0.0, -18.0), Vector3(12.0, 5.0, 10.0), Color(0.66, 0.60, 0.46))
	_add_building(Vector3(-20.0, 0.0, -8.0), Vector3(7.0, 4.0, 7.0), Color(0.52, 0.58, 0.62))
	_add_building(Vector3(19.0, 0.0, -12.0), Vector3(8.0, 6.0, 8.0), Color(0.62, 0.52, 0.40))
	_add_building(Vector3(-13.0, 0.0, 17.0), Vector3(9.0, 3.5, 7.0), Color(0.58, 0.62, 0.48))
	_add_building(Vector3(17.0, 0.0, 18.0), Vector3(6.0, 4.0, 9.0), Color(0.70, 0.66, 0.55))
	for i in range(18):
		var x := float((i * 17) % 90) - 45.0
		var z := float((i * 29) % 90) - 45.0
		if Vector2(x, z).length() > 13.0:
			_add_tree(Vector3(x, 0.0, z))

func _add_building(pos: Vector3, dimensions: Vector3, color: Color) -> void:
	var building := StaticBody3D.new()
	building.position = pos
	var mesh_instance := MeshInstance3D.new()
	var mesh := BoxMesh.new()
	mesh.size = dimensions
	mesh_instance.mesh = mesh
	var material := StandardMaterial3D.new()
	material.albedo_color = color
	material.roughness = 0.9
	mesh_instance.material_override = material
	mesh_instance.position.y = dimensions.y * 0.5
	building.add_child(mesh_instance)
	var collision := CollisionShape3D.new()
	var shape := BoxShape3D.new()
	shape.size = dimensions
	collision.shape = shape
	collision.position.y = dimensions.y * 0.5
	building.add_child(collision)
	add_child(building)

func _add_tree(pos: Vector3) -> void:
	var trunk := MeshInstance3D.new()
	var trunk_mesh := CylinderMesh.new()
	trunk_mesh.top_radius = 0.22
	trunk_mesh.bottom_radius = 0.34
	trunk_mesh.height = 2.8
	trunk.mesh = trunk_mesh
	trunk.position = pos + Vector3(0.0, 1.4, 0.0)
	var trunk_material := StandardMaterial3D.new()
	trunk_material.albedo_color = Color(0.28, 0.18, 0.10)
	trunk.material_override = trunk_material
	add_child(trunk)

	var crown := MeshInstance3D.new()
	var crown_mesh := SphereMesh.new()
	crown_mesh.radius = 1.35
	crown_mesh.height = 2.7
	crown.mesh = crown_mesh
	crown.position = pos + Vector3(0.0, 3.2, 0.0)
	var crown_material := StandardMaterial3D.new()
	crown_material.albedo_color = Color(0.12, 0.30, 0.16)
	crown_material.roughness = 1.0
	crown.material_override = crown_material
	add_child(crown)

func _spawn_player() -> void:
	var player := CharacterBody3D.new()
	player.name = "Player"
	player.set_script(PLAYER_SCRIPT)
	player.position = Vector3(0.0, 0.5, 8.0)
	add_child(player)

func _build_hud() -> void:
	var canvas := CanvasLayer.new()
	add_child(canvas)
	var label := Label.new()
	label.text = "ECOS • PROTOTIPE OFFLINE\nWASD bergerak | Mouse melihat | Space lompat | Esc lepas mouse"
	label.position = Vector2(18.0, 18.0)
	label.add_theme_font_size_override("font_size", 16)
	canvas.add_child(label)

	var back := Button.new()
	back.text = "Kembali ke Studio"
	back.position = Vector2(18.0, 76.0)
	back.pressed.connect(func(): get_tree().change_scene_to_file("res://scenes/studio.tscn"))
	canvas.add_child(back)
