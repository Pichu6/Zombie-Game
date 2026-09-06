extends SceneTree
func _initialize() -> void:
	var root_node = Node3D.new()
	root_node.name = "BackpackSchoolPrototype"
	var canvas = load("res://assets/items/backpack_school/canvas.tres")
	var dark = StandardMaterial3D.new()
	dark.albedo_color = Color(0.035, 0.055, 0.047)
	dark.roughness = 0.8
	var metal = StandardMaterial3D.new()
	metal.albedo_color = Color(0.35, 0.40, 0.34)
	metal.metallic = 0.6
	for spec in [
		["Shell", Vector3(0,0,0), Vector3(0.90,1,0.55), 0.19, 0.46, canvas],
		["FrontPocket", Vector3(0,-0.09,-0.09), Vector3(1,0.8,0.4), 0.14, 0.28, canvas],
		["TopFlap", Vector3(0,0.16,-0.015), Vector3(1,0.25,0.58), 0.18, 0.36, dark]]:
		var node = MeshInstance3D.new()
		node.name = spec[0]
		var dimensions = Vector3(0.34, 0.44, 0.18)
		if spec[0] == "FrontPocket": dimensions = Vector3(0.27, 0.20, 0.07)
		if spec[0] == "TopFlap": dimensions = Vector3(0.35, 0.065, 0.20)
		node.mesh = _rounded_box(dimensions, minf(dimensions.y, dimensions.z) * 0.3)
		node.material_override = spec[5]
		node.position = spec[1]
		node.scale = Vector3.ONE
		root_node.add_child(node)
		node.owner = root_node
	for side in [-1, 1]:
		for spec in [["ShoulderStrap", Vector3(side*0.10,0.02,0.085), Vector3(0.032,0.33,0.025), dark], ["Buckle", Vector3(side*0.075,-0.045,-0.153), Vector3(0.028,0.04,0.015), metal], ["PocketWebbing",Vector3(side*0.075,-0.085,-0.15),Vector3(0.02,0.18,0.014),dark]]:
			var node = MeshInstance3D.new()
			node.name = str(spec[0]) + str(side)
			var mesh = BoxMesh.new()
			mesh.size = spec[2]
			node.mesh = mesh
			node.position = spec[1]
			node.material_override = spec[3]
			root_node.add_child(node)
			node.owner = root_node
	var scene = PackedScene.new()
	scene.pack(root_node)
	var result = ResourceSaver.save(scene, "res://assets/items/backpack_school/model.tscn")
	root_node.free()
	print("BACKPACK_GENERATED ", result)
	quit(result)

func _rounded_box(dimensions: Vector3, radius: float) -> ArrayMesh:
	var surface = SurfaceTool.new()
	surface.begin(Mesh.PRIMITIVE_TRIANGLES)
	var half = dimensions * 0.5
	var inner = half - Vector3.ONE * radius
	for normal in [Vector3.RIGHT, Vector3.LEFT, Vector3.UP, Vector3.DOWN, Vector3.FORWARD, Vector3.BACK]:
		var u = normal.cross(Vector3.UP) if absf(normal.y) < 0.5 else normal.cross(Vector3.RIGHT)
		var v = normal.cross(u)
		var extent = normal.abs().dot(half)
		for row in range(8):
			for col in range(8):
				for corner in [Vector2(0,0),Vector2(1,1),Vector2(1,0),Vector2(0,0),Vector2(0,1),Vector2(1,1)]:
					var uv = (Vector2(col, row) + corner) / 8.0
					var point = normal * extent + u * ((uv.x*2-1) * u.abs().dot(half)) + v * ((uv.y*2-1) * v.abs().dot(half))
					var core = point.clamp(-inner, inner)
					var n = (point - core).normalized()
					surface.set_normal(n)
					surface.set_uv(uv)
					surface.add_vertex(core + n * radius)
	surface.index()
	return surface.commit()