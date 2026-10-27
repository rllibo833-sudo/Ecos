extends Control

const WORLD_SCENE := "res://scenes/world.tscn"

func _ready() -> void:
	_build_ui()

func _build_ui() -> void:
	var background := ColorRect.new()
	background.color = Color(0.035, 0.055, 0.085, 1.0)
	background.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(background)

	var layout := VBoxContainer.new()
	layout.set_anchors_and_offsets_preset(Control.PRESET_CENTER)
	layout.offset_left = -250
	layout.offset_top = -170
	layout.offset_right = 250
	layout.offset_bottom = 170
	layout.add_theme_constant_override("separation", 18)
	add_child(layout)

	var title := Label.new()
	title.text = "ECOS  /  GAME STUDIO"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title.add_theme_font_size_override("font_size", 34)
	layout.add_child(title)

	var subtitle := Label.new()
	subtitle.text = "Offline 3D Prototype  •  Godot"
	subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	subtitle.add_theme_font_size_override("font_size", 16)
	layout.add_child(subtitle)

	var description := Label.new()
	description.text = "Prototipe pertama ECOS. Jalankan dunia 3D lokal tanpa koneksi internet."
	description.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	description.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	layout.add_child(description)

	var launch := Button.new()
	launch.text = "JALANKAN GAME OFFLINE"
	launch.custom_minimum_size = Vector2(300, 56)
	launch.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	launch.pressed.connect(_launch_game)
	layout.add_child(launch)

	var status := Label.new()
	status.text = "WASD: bergerak  •  Mouse: melihat  •  Space: lompat  •  Esc: lepas mouse"
	status.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	layout.add_child(status)

func _launch_game() -> void:
	var result := get_tree().change_scene_to_file(WORLD_SCENE)
	if result != OK:
		push_error("Gagal membuka dunia ECOS: %s" % result)
