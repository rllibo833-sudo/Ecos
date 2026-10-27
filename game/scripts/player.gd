extends CharacterBody3D

@export var move_speed: float = 6.0
@export var jump_velocity: float = 5.0
@export var mouse_sensitivity: float = 0.0025

var gravity: float = 18.0
var camera_pivot: Node3D
var camera: Camera3D
var pitch: float = 0.0

func _ready() -> void:
	_add_input_actions()
	_build_body()
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		rotate_y(-event.relative.x * mouse_sensitivity)
		pitch = clampf(pitch - event.relative.y * mouse_sensitivity, -1.2, 1.2)
		camera_pivot.rotation.x = pitch
	if event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
		Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _physics_process(delta: float) -> void:
	if not is_on_floor():
		velocity.y -= gravity * delta
	if Input.is_action_just_pressed("ecos_jump") and is_on_floor():
		velocity.y = jump_velocity

	var input_vector := Input.get_vector("ecos_left", "ecos_right", "ecos_forward", "ecos_back")
	var direction := (global_transform.basis * Vector3(input_vector.x, 0.0, input_vector.y)).normalized()
	if direction:
		velocity.x = direction.x * move_speed
		velocity.z = direction.z * move_speed
	else:
		velocity.x = move_toward(velocity.x, 0.0, move_speed * 5.0 * delta)
		velocity.z = move_toward(velocity.z, 0.0, move_speed * 5.0 * delta)
	move_and_slide()

func _build_body() -> void:
	var collision := CollisionShape3D.new()
	var capsule := CapsuleShape3D.new()
	capsule.radius = 0.38
	capsule.height = 1.8
	collision.shape = capsule
	collision.position.y = 0.9
	add_child(collision)

	var body_mesh := MeshInstance3D.new()
	var capsule_mesh := CapsuleMesh.new()
	capsule_mesh.radius = 0.38
	capsule_mesh.height = 1.8
	body_mesh.mesh = capsule_mesh
	body_mesh.position.y = 0.9
	var material := StandardMaterial3D.new()
	material.albedo_color = Color(0.12, 0.32, 0.48)
	material.roughness = 0.78
	body_mesh.material_override = material
	add_child(body_mesh)

	camera_pivot = Node3D.new()
	camera_pivot.position.y = 1.55
	add_child(camera_pivot)
	camera = Camera3D.new()
	camera.position = Vector3(0.0, 0.15, 4.2)
	camera.rotation.x = deg_to_rad(-8.0)
	camera.current = true
	camera_pivot.add_child(camera)

func _add_input_actions() -> void:
	_bind_key("ecos_forward", KEY_W)
	_bind_key("ecos_back", KEY_S)
	_bind_key("ecos_left", KEY_A)
	_bind_key("ecos_right", KEY_D)
	_bind_key("ecos_jump", KEY_SPACE)

func _bind_key(action_name: String, key: Key) -> void:
	if not InputMap.has_action(action_name):
		InputMap.add_action(action_name)
	var event := InputEventKey.new()
	event.physical_keycode = key
	InputMap.action_add_event(action_name, event)
