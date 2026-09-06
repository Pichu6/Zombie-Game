@tool
extends SceneTree

func _init():
	var p = 'res://char_human/rig/makehuman/model normal.fbx'
	var inst = load(p).instantiate()
	for child in inst.find_children('*', '', true, false):
		print('Node: ', child.name, ' type: ', child.get_class())
		if child is MeshInstance3D:
			print('  Mesh: ', child.mesh.resource_name, ' AABB: ', child.get_aabb())
			for s in range(child.mesh.get_surface_count()):
				print('    Surface ', s, ' name: ', child.mesh.surface_get_material(s))
	inst.free()
	quit()
