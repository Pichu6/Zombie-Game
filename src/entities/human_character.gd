class_name HumanCharacter
extends Node3D

const BodySystemClass = preload("res://src/biology/body_system.gd")
const MasterLoader = preload("res://src/entities/character_master_loader.gd")

signal biometrics_changed(height: float, muscle: float, fat: float, weight: float)
signal animation_state_changed(anim_name: String)
signal archetype_changed(new_archetype: String)

var body: BodySystemClass
var visual_root: Node3D

var current_model: Node3D
var active_archetype: String = "normal"
var anim_player: AnimationPlayer
var anim_tree: AnimationTree
var playback: AnimationNodeStateMachinePlayback
var skeleton: Skeleton3D

var simulation_speed: float = 1.0
var is_collapsed: bool = false
var is_crouched: bool = false

var target_effort: float = 0.0
var current_effort: float = 0.0
var active_speed_scale: float = 1.0
var current_anim_state: String = "Idle"

var current_fat: float = 0.18
var current_muscle: float = 0.45
var current_height: float = 1.78

var walk_len: float = 1.033
var run_len: float = 0.633
var sprint_len: float = 0.533

var sensory_bubble_3d: Label3D
var show_3d_sensory_bubble: bool = true

func _ready() -> void:
	body = BodySystemClass.new()
	body.thermo.clothing_insulation_clo = 0.0 # Malla 3D base descalza y sin ropa
	body.player_collapsed.connect(_on_player_collapsed)
	visual_root = get_node_or_null("VisualRoot")
	if visual_root:
		_setup_character_model()
		set_character_archetype("normal")
	_setup_sensory_bubble()

func _setup_sensory_bubble() -> void:
	sensory_bubble_3d = Label3D.new()
	sensory_bubble_3d.name = "SensoryBubble3D"
	sensory_bubble_3d.billboard = BaseMaterial3D.BILLBOARD_ENABLED
	sensory_bubble_3d.no_depth_test = true
	sensory_bubble_3d.position = Vector3(0, 2.05, 0)
	sensory_bubble_3d.pixel_size = 0.0030
	sensory_bubble_3d.font_size = 20
	sensory_bubble_3d.outline_size = 8
	sensory_bubble_3d.outline_modulate = Color(0.04, 0.05, 0.08, 0.95)
	sensory_bubble_3d.modulate = Color(0.95, 0.95, 0.95, 0.95)
	sensory_bubble_3d.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	sensory_bubble_3d.vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM
	sensory_bubble_3d.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	sensory_bubble_3d.width = 480.0
	add_child(sensory_bubble_3d)

func set_sensory_bubble_visible(is_visible: bool) -> void:
	show_3d_sensory_bubble = is_visible
	if sensory_bubble_3d:
		sensory_bubble_3d.visible = is_visible

func toggle_sensory_bubble() -> void:
	set_sensory_bubble_visible(not show_3d_sensory_bubble)

func _setup_character_model() -> void:
	for child in visual_root.get_children():
		child.queue_free()
		
	current_model = MasterLoader.load_character_model()
	if not current_model:
		push_error("HumanCharacter: Failed to load character model")
		return
		
	current_model.name = "Model_Master"
	visual_root.add_child(current_model)
	
	skeleton = current_model.find_child("Skeleton3D", true, false)
	anim_player = current_model.find_child("AnimationPlayer", true, false)
	if not anim_player:
		anim_player = AnimationPlayer.new()
		anim_player.name = "AnimationPlayer"
		current_model.add_child(anim_player)
	anim_player.root_node = anim_player.get_path_to(current_model)
	
	_fix_mesh_skeletons(current_model, skeleton)
	_setup_model_animations(current_model, skeleton, anim_player)
	anim_tree = _setup_model_animation_tree(current_model, anim_player)
	
	playback = anim_tree.get("parameters/playback") if anim_tree else null
	if playback:
		playback.start(&"Idle")

func _fix_mesh_skeletons(model: Node3D, skel: Skeleton3D) -> void:
	for mesh in model.find_children("*", "MeshInstance3D", true, false):
		if mesh is MeshInstance3D:
			if mesh.get_parent() is Skeleton3D:
				mesh.skeleton = NodePath("..")
			elif skel:
				mesh.skeleton = mesh.get_path_to(skel)

func _setup_model_animations(model: Node3D, skel: Skeleton3D, ap: AnimationPlayer) -> void:
	var lib: AnimationLibrary
	if ap.has_animation_library(""):
		lib = ap.get_animation_library("")
		for anim_name in lib.get_animation_list():
			lib.remove_animation(anim_name)
	else:
		lib = AnimationLibrary.new()
		ap.add_animation_library("", lib)

	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Idle.fbx", "Idle", Animation.LOOP_LINEAR)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Walking.fbx", "Walking", Animation.LOOP_LINEAR)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Running.fbx", "Running", Animation.LOOP_LINEAR)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Sprint.fbx", "Sprint", Animation.LOOP_LINEAR)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Standing To Crouched.fbx", "Crouch_Down", Animation.LOOP_NONE)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Crouch To Stand.fbx", "Crouch_Up", Animation.LOOP_NONE)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Fall Flat.fbx", "Fall_Flat", Animation.LOOP_NONE)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Falling Forward Death.fbx", "Death_Forward", Animation.LOOP_NONE)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Standing React Death Forward.fbx", "Death_Forward_React", Animation.LOOP_NONE)
	_load_fbx_anim_into_player(model, skel, ap, "res://char_human/anim/Standing React Death Backward.fbx", "Death_Backward", Animation.LOOP_NONE)

func _load_fbx_anim_into_player(model: Node3D, skel: Skeleton3D, ap: AnimationPlayer, fbx_path: String, target_name: String, loop_mode: Animation.LoopMode) -> void:
	if not ResourceLoader.exists(fbx_path):
		return
	var res = load(fbx_path)
	if not res:
		return
	var inst = res.instantiate()
	var src_ap: AnimationPlayer = inst.find_child("AnimationPlayer", true, false)
	if src_ap:
		var anim_name = "mixamo_com" if src_ap.has_animation("mixamo_com") else ("Take 001" if src_ap.has_animation("Take 001") else "")
		if anim_name != "":
			var anim: Animation = src_ap.get_animation(anim_name).duplicate()
			anim.loop_mode = loop_mode
			_retarget_anim(model, skel, anim, target_name)
			var lib: AnimationLibrary = ap.get_animation_library("")
			if lib and not ap.has_animation(target_name):
				lib.add_animation(target_name, anim)
	inst.queue_free()

func _retarget_anim(model: Node3D, skel: Skeleton3D, anim: Animation, target_name: String) -> void:
	if not skel or not model:
		return
	var skel_rel_path: String = str(model.get_path_to(skel))
	var tracks_to_remove: Array[int] = []

	for t in range(anim.get_track_count()):
		var t_type = anim.track_get_type(t)
		if t_type == Animation.TYPE_SCALE_3D:
			tracks_to_remove.append(t)
			continue
			
		var old_path: String = str(anim.track_get_path(t))
		var colon_idx: int = old_path.rfind(":")
		var raw_bone: String = old_path.substr(colon_idx + 1) if colon_idx >= 0 else old_path
		var clean_name: String = raw_bone.replace("mixamorig1:", "").replace("mixamorig:", "").replace("mixamorig1_", "").replace("mixamorig_", "")

		var found_bone: int = skel.find_bone("mixamorig_" + clean_name)
		if found_bone < 0:
			found_bone = skel.find_bone(clean_name)
		if found_bone < 0:
			found_bone = skel.find_bone(raw_bone)

		if found_bone >= 0:
			var actual_name = skel.get_bone_name(found_bone)
			var is_hips: bool = (clean_name.to_lower() == "hips" or "hip" in actual_name.to_lower())
			
			if t_type == Animation.TYPE_POSITION_3D:
				if not is_hips:
					tracks_to_remove.append(t)
					continue
					
				var k_count = anim.track_get_key_count(t)
				if k_count > 0:
					var first_pos: Vector3 = anim.track_get_key_value(t, 0)
					for k in range(k_count):
						var pos: Vector3 = anim.track_get_key_value(t, k)
						anim.track_set_key_value(t, k, Vector3(first_pos.x, pos.y, first_pos.z))
						
			anim.track_set_path(t, NodePath(skel_rel_path + ":" + actual_name))
		else:
			tracks_to_remove.append(t)

	for i in range(tracks_to_remove.size() - 1, -1, -1):
		anim.remove_track(tracks_to_remove[i])

func _setup_model_animation_tree(model: Node3D, ap: AnimationPlayer) -> AnimationTree:
	var at = AnimationTree.new()
	at.name = "AnimationTree"

	var state_machine = AnimationNodeStateMachine.new()

	var node_idle = AnimationNodeAnimation.new()
	node_idle.animation = &"Idle"
	state_machine.add_node(&"Idle", node_idle)

	var states_with_speed = ["Walking", "Running", "Sprint"]
	for s in states_with_speed:
		var anim_node = AnimationNodeAnimation.new()
		anim_node.animation = StringName(s)

		var ts_node = AnimationNodeTimeScale.new()
		var btree = AnimationNodeBlendTree.new()
		btree.add_node("anim", anim_node)
		btree.add_node("speed", ts_node)
		btree.connect_node("speed", 0, "anim")
		btree.connect_node("output", 0, "speed")

		state_machine.add_node(StringName(s), btree)

	var loco_states = [&"Idle", &"Walking", &"Running", &"Sprint"]
	for from_s in loco_states:
		for to_s in loco_states:
			if from_s != to_s:
				var t = AnimationNodeStateMachineTransition.new()
				t.xfade_time = 0.32
				state_machine.add_transition(from_s, to_s, t)

	var node_crouch_down = AnimationNodeAnimation.new()
	node_crouch_down.animation = &"Crouch_Down"
	state_machine.add_node(&"Crouch_Down", node_crouch_down)

	var node_crouch_up = AnimationNodeAnimation.new()
	node_crouch_up.animation = &"Crouch_Up"
	state_machine.add_node(&"Crouch_Up", node_crouch_up)

	var node_fall_flat = AnimationNodeAnimation.new()
	node_fall_flat.animation = &"Fall_Flat"
	state_machine.add_node(&"Fall_Flat", node_fall_flat)

	var node_death_bwd = AnimationNodeAnimation.new()
	node_death_bwd.animation = &"Death_Backward"
	state_machine.add_node(&"Death_Backward", node_death_bwd)

	var node_death_fwd = AnimationNodeAnimation.new()
	node_death_fwd.animation = &"Death_Forward_React"
	state_machine.add_node(&"Death_Forward_React", node_death_fwd)

	var t_crouch_up_to_idle = AnimationNodeStateMachineTransition.new()
	t_crouch_up_to_idle.switch_mode = AnimationNodeStateMachineTransition.SWITCH_MODE_AT_END
	t_crouch_up_to_idle.advance_mode = AnimationNodeStateMachineTransition.ADVANCE_MODE_AUTO
	t_crouch_up_to_idle.xfade_time = 0.35
	state_machine.add_transition(&"Crouch_Up", &"Idle", t_crouch_up_to_idle)

	for s in loco_states:
		var t_c = AnimationNodeStateMachineTransition.new()
		t_c.xfade_time = 0.25
		state_machine.add_transition(s, &"Crouch_Down", t_c)

		var t_f = AnimationNodeStateMachineTransition.new()
		t_f.xfade_time = 0.15
		state_machine.add_transition(s, &"Fall_Flat", t_f)

		var t_b = AnimationNodeStateMachineTransition.new()
		t_b.xfade_time = 0.25
		state_machine.add_transition(s, &"Death_Backward", t_b)

		var t_df = AnimationNodeStateMachineTransition.new()
		t_df.xfade_time = 0.25
		state_machine.add_transition(s, &"Death_Forward_React", t_df)

	var t_cd_cu = AnimationNodeStateMachineTransition.new()
	t_cd_cu.xfade_time = 0.25
	state_machine.add_transition(&"Crouch_Down", &"Crouch_Up", t_cd_cu)

	var death_states = [&"Fall_Flat", &"Death_Backward", &"Death_Forward_React"]
	for d in death_states:
		var t_cd_d = AnimationNodeStateMachineTransition.new()
		t_cd_d.xfade_time = 0.20
		state_machine.add_transition(&"Crouch_Down", d, t_cd_d)

	var t_f_i = AnimationNodeStateMachineTransition.new()
	t_f_i.xfade_time = 0.35
	state_machine.add_transition(&"Fall_Flat", &"Idle", t_f_i)

	var t_b_i = AnimationNodeStateMachineTransition.new()
	t_b_i.xfade_time = 0.35
	state_machine.add_transition(&"Death_Backward", &"Idle", t_b_i)

	var t_df_i = AnimationNodeStateMachineTransition.new()
	t_df_i.xfade_time = 0.35
	state_machine.add_transition(&"Death_Forward_React", &"Idle", t_df_i)

	at.tree_root = state_machine
	model.add_child(at)
	at.anim_player = at.get_path_to(ap)
	at.active = true
	return at

func set_character_archetype(p_archetype: String) -> void:
	var arch = p_archetype.to_lower()
	if not body:
		return

	active_archetype = arch

	if arch == "skinny":
		body.height_meters = 1.76
		body.body_fat_pct = 0.04
		body.muscle_mass_pct = 0.20
	elif arch == "fat":
		body.height_meters = 1.78
		body.body_fat_pct = 0.85
		body.muscle_mass_pct = 0.35
	elif arch == "muscle":
		body.height_meters = 1.80
		body.body_fat_pct = 0.12
		body.muscle_mass_pct = 0.85
	else: # normal
		body.height_meters = 1.78
		body.body_fat_pct = 0.18
		body.muscle_mass_pct = 0.45

	body.recalculate_weight()
	apply_biometrics(body.height_meters, body.muscle_mass_pct, body.body_fat_pct)
	archetype_changed.emit(active_archetype)

func apply_biometrics(p_height: float, p_muscle: float, p_fat: float) -> void:
	if not body:
		return
	body.height_meters = p_height
	body.muscle_mass_pct = p_muscle
	body.body_fat_pct = p_fat
	body.recalculate_weight()

	biometrics_changed.emit(body.height_meters, body.muscle_mass_pct, body.body_fat_pct, body.total_weight_kg)

func _process(delta: float) -> void:
	preload("res://src/items/equipment_visuals.gd").sync(self)
	if body:
		body.update(delta * simulation_speed)

		# Silky-smooth GPU blend shape morphing towards target biometrics
		var t_rate: float = 1.0 - exp(-12.0 * delta)
		current_fat = lerp(current_fat, body.body_fat_pct, t_rate)
		current_muscle = lerp(current_muscle, body.muscle_mass_pct, t_rate)
		current_height = lerp(current_height, body.height_meters, t_rate)

		MasterLoader.apply_morph_weights(current_model, current_fat, current_muscle)

		var ref_height = 1.78
		if visual_root:
			visual_root.scale = Vector3(1.0, current_height / ref_height, 1.0)

		if sensory_bubble_3d and sensory_bubble_3d.visible:
			var rep = body.get_sensory_report()
			sensory_bubble_3d.text = "💭 %s\n«%s»" % [rep.mood_tag, rep.primary_thought]
			sensory_bubble_3d.modulate = rep.thought_color
			sensory_bubble_3d.position.y = (current_height * 1.05) + 0.18

	if anim_tree and anim_tree.active and not is_collapsed:
		if not is_crouched:
			var effort_decay: float = 1.0 - exp(-5.0 * delta)
			current_effort = lerp(current_effort, target_effort, effort_decay)

			var target_speed: float = 1.0

			if current_effort <= 0.02:
				if playback and playback.get_current_node() != "Idle" and playback.get_current_node() != "Crouch_Up":
					playback.travel(&"Idle")
				target_speed = 1.0
			elif current_effort <= 0.35:
				if playback and playback.get_current_node() != "Walking":
					playback.travel(&"Walking")
				var norm = (current_effort - 0.02) / 0.33
				var cadence = lerp(1.40, 2.40, norm)
				target_speed = cadence / (2.0 / walk_len)
				active_speed_scale = lerp(active_speed_scale, target_speed, 1.0 - exp(-7.0 * delta))
				anim_tree.set("parameters/Walking/speed/scale", active_speed_scale)
			elif current_effort <= 0.70:
				if playback and playback.get_current_node() != "Running":
					playback.travel(&"Running")
				var norm = (current_effort - 0.35) / 0.35
				var cadence = lerp(2.40, 3.80, norm)
				target_speed = cadence / (2.0 / run_len)
				active_speed_scale = lerp(active_speed_scale, target_speed, 1.0 - exp(-7.0 * delta))
				anim_tree.set("parameters/Running/speed/scale", active_speed_scale)
			else:
				if playback and playback.get_current_node() != "Sprint":
					playback.travel(&"Sprint")
				var norm = (current_effort - 0.70) / 0.30
				var cadence = lerp(3.80, 4.70, norm)
				target_speed = cadence / (2.0 / sprint_len)
				active_speed_scale = lerp(active_speed_scale, target_speed, 1.0 - exp(-7.0 * delta))
				anim_tree.set("parameters/Sprint/speed/scale", active_speed_scale)

		var cur_node = str(playback.get_current_node()) if playback else ""
		if cur_node != current_anim_state:
			current_anim_state = cur_node
			animation_state_changed.emit(cur_node)

func set_character_posture(posture_val: int) -> void:
	if not body:
		return
	body.set_posture(posture_val as BodySystemClass.Posture)
	match posture_val:
		BodySystemClass.Posture.STANDING:
			is_crouched = false
			is_collapsed = false
			if playback and current_effort <= 0.02:
				playback.travel(&"Idle")
		BodySystemClass.Posture.SITTING:
			is_crouched = false
			is_collapsed = false
			set_locomotion_effort(0.0)
			if playback:
				playback.travel(&"Idle")
		BodySystemClass.Posture.LYING:
			is_crouched = false
			set_locomotion_effort(0.0)
			if playback:
				playback.travel(&"Fall_Flat")
		BodySystemClass.Posture.CROUCHING:
			is_crouched = true
			set_locomotion_effort(0.0)
			if playback:
				playback.travel(&"Crouch_Down")

func set_locomotion_effort(effort_norm: float) -> void:
	if not playback or is_collapsed:
		return

	if effort_norm > 0.02:
		if is_crouched:
			is_crouched = false
		if body and body.current_posture != BodySystemClass.Posture.STANDING:
			body.set_posture(BodySystemClass.Posture.STANDING)

	target_effort = clamp(effort_norm, 0.0, 1.0)
	if body:
		body.exertion_factor = target_effort

func play_idle() -> void:
	is_collapsed = false
	is_crouched = false
	target_effort = 0.0
	current_effort = 0.0
	if body:
		body.set_posture(BodySystemClass.Posture.STANDING)
		body.exertion_factor = 0.0
	if playback:
		playback.travel(&"Idle")

func reset_character_state() -> void:
	is_collapsed = false
	is_crouched = false
	target_effort = 0.0
	current_effort = 0.0
	active_speed_scale = 1.0
	current_anim_state = "Idle"
	if playback:
		playback.start(&"Idle")
	if body:
		body.reset_to_base_state()

func play_crouch_down() -> void:
	is_crouched = true
	target_effort = 0.0
	current_effort = 0.0
	if body:
		body.set_posture(BodySystemClass.Posture.CROUCHING)
		body.exertion_factor = 0.0
	if playback:
		playback.travel(&"Crouch_Down")

func play_crouch_up() -> void:
	is_crouched = false
	if body:
		body.set_posture(BodySystemClass.Posture.STANDING)
	if playback:
		playback.travel(&"Crouch_Up")

func play_fall_flat() -> void:
	is_collapsed = true
	is_crouched = false
	if playback:
		playback.travel(&"Fall_Flat")

func play_death_bwd() -> void:
	is_collapsed = true
	is_crouched = false
	if playback:
		playback.travel(&"Death_Backward")

func play_death_fwd() -> void:
	is_collapsed = true
	is_crouched = false
	if playback:
		playback.travel(&"Death_Forward_React")

func _on_player_collapsed(reason: String) -> void:
	is_collapsed = true
	is_crouched = false
	target_effort = 0.0
	if playback:
		if reason == "CARDIOVASCULAR_COLLAPSE" or reason == "HEATSTROKE":
			playback.travel(&"Fall_Flat")
		elif reason == "MASSIVE_HEMORRHAGE":
			playback.travel(&"Death_Backward")
		else:
			playback.travel(&"Death_Forward_React")
