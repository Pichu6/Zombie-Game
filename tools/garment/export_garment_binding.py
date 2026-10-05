"""Exports the garment-to-body "binding" for our own engine (Blender 4.x / 5.x).

Blender is only an authoring tool here. The engine does not read garment shape
keys: it reads this binding and, every frame, places each garment vertex on the
already deformed body (morphs + skeleton). See wrap_reference.py.

Usage:
  blender scene.blend --background --factory-startup --python export_garment_binding.py -- \
      --body Body --collection Garments --out-dir export/

Optional vertex groups on each garment:
  fit_follow  0..1  how much it follows body shape (1 = tight; ~0.5 = loose).
  fit_rigid   >0.5  islands that move as one rigid piece (buckles, pockets,
                    backpack frame, sole). They share the anchor at their centre.

Output per garment: <name>.bind.json (header) + <name>.bind.bin (little-endian,
consecutive arrays in "layout" order). Also body.tris.bin with the body's
triangles in the order tri_id refers to.
"""
import argparse
import json
import os
import struct
import sys

import bpy
from mathutils import Vector
from mathutils.bvhtree import BVHTree
from mathutils.geometry import barycentric_transform

NO_ISLAND = 0xFFFFFFFF


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    p = argparse.ArgumentParser()
    p.add_argument("--body", required=True)
    p.add_argument("--collection", required=True)
    p.add_argument("--out-dir", required=True)
    return p.parse_args(argv)


class BodySurface:
    """Body at rest (all keys at 0), triangulated, in world space."""

    def __init__(self, body):
        if body.data.shape_keys:
            for k in body.data.shape_keys.key_blocks:
                k.value = 0.0
        bpy.context.view_layer.update()
        mesh = body.data
        mesh.calc_loop_triangles()
        mw = body.matrix_world
        self.verts = [mw @ v.co for v in mesh.vertices]
        self.tris = [tuple(t.vertices) for t in mesh.loop_triangles]
        uv = mesh.uv_layers.active
        self.tri_uvs = [[uv.data[l].uv.copy() for l in t.loops] if uv else [Vector((0, 0))] * 3
                        for t in mesh.loop_triangles]
        self.normals = vertex_normals(self.verts, self.tris)
        self.weights = [[(g.group, g.weight) for g in v.groups] for v in mesh.vertices]
        self.group_names = [g.name for g in body.vertex_groups]
        self.bvh = BVHTree.FromPolygons(self.verts, self.tris)

    def frame(self, tri, bary):
        """Point and smooth (T, B, N) frame on the triangle. Same maths as the engine."""
        a, b, c = (self.verts[i] for i in self.tris[tri])
        na, nb, nc = (self.normals[i] for i in self.tris[tri])
        u, v = bary
        w = 1.0 - u - v
        p = a * w + b * u + c * v
        n = (na * w + nb * u + nc * v).normalized()
        e = b - a
        t = (e - n * e.dot(n)).normalized()
        return p, t, n.cross(t), n

    def bind_point(self, co):
        loc, _normal, tri, _dist = self.bvh.find_nearest(co)
        a, b, c = (self.verts[i] for i in self.tris[tri])
        bc = barycentric_transform(loc, a, b, c, Vector((0, 0, 0)), Vector((1, 0, 0)), Vector((0, 1, 0)))
        return tri, (bc.x, bc.y)


def vertex_normals(verts, tris):
    acc = [Vector() for _ in verts]
    for i, j, k in tris:
        n = (verts[j] - verts[i]).cross(verts[k] - verts[i])  # area-weighted
        for idx in (i, j, k):
            acc[idx] += n
    return [n.normalized() for n in acc]


def group_weights(obj, name, default):
    vg = obj.vertex_groups.get(name)
    out = [default] * len(obj.data.vertices)
    if vg:
        for v in obj.data.vertices:
            for g in v.groups:
                if g.group == vg.index:
                    out[v.index] = g.weight
    return out


def rigid_islands(obj, rigid):
    adj = {i: [] for i, w in enumerate(rigid) if w > 0.5}
    for e in obj.data.edges:
        a, b = e.vertices
        if a in adj and b in adj:
            adj[a].append(b)
            adj[b].append(a)
    seen, islands = set(), []
    for start in adj:
        if start in seen:
            continue
        stack, island = [start], []
        seen.add(start)
        while stack:
            i = stack.pop()
            island.append(i)
            for j in adj[i]:
                if j not in seen:
                    seen.add(j)
                    stack.append(j)
        islands.append(island)
    return islands


def bind_garment(garment, body):
    mw = garment.matrix_world
    verts = [mw @ v.co for v in garment.data.vertices]
    n = len(verts)
    follow = group_weights(garment, "fit_follow", 1.0)
    islands = rigid_islands(garment, group_weights(garment, "fit_rigid", 0.0))
    anchor = list(range(n))  # each vertex anchors to its own point...
    island_of = [NO_ISLAND] * n
    for k, island in enumerate(islands):  # ...except rigid islands, which share one anchor
        centre = sum((verts[i] for i in island), Vector()) / len(island)
        nearest = min(island, key=lambda i: (verts[i] - centre).length)
        for i in island:
            anchor[i] = nearest
            island_of[i] = k

    tri_id, bary, offset, body_uv, bones, bone_w = [], [], [], [], [], []
    cache = {}
    for i, co in enumerate(verts):
        a = anchor[i]
        if a not in cache:
            cache[a] = body.bind_point(verts[a])
        tri, bc = cache[a]
        p, t, b, nrm = body.frame(tri, bc)
        d = co - p
        tri_id.append(tri)
        bary += [bc[0], bc[1]]
        offset += [d.dot(t), d.dot(b), d.dot(nrm)]
        u, v = bc
        uvs = body.tri_uvs[tri]
        uv = uvs[0] * (1 - u - v) + uvs[1] * u + uvs[2] * v
        body_uv += [uv.x, uv.y]
        # Bone weights: interpolated from the body, top 4, normalised.
        acc = {}
        for corner, wc in zip(body.tris[tri], (1 - u - v, u, v)):
            for g, w in body.weights[corner]:
                acc[g] = acc.get(g, 0.0) + w * wc
        top = sorted(acc.items(), key=lambda kv: -kv[1])[:4]
        total = sum(w for _, w in top) or 1.0
        top += [(0, 0.0)] * (4 - len(top))
        bones += [g for g, _ in top]
        bone_w += [w / total for _, w in top]
    return {
        "tri_id": ("I", tri_id), "bary": ("f", bary), "offset_tbn": ("f", offset),
        "follow": ("f", follow), "island": ("I", island_of), "body_uv": ("f", body_uv),
        "bone_index": ("H", bones), "bone_weight": ("f", bone_w),
    }, len(islands)


def write(path_base, arrays, meta):
    layout = []
    with open(path_base + ".bin", "wb") as f:
        for name, (fmt, data) in arrays.items():
            f.write(struct.pack("<%d%s" % (len(data), fmt), *data))
            layout.append({"name": name, "type": {"I": "u32", "f": "f32", "H": "u16"}[fmt],
                           "count": len(data)})
    meta["layout"] = layout
    with open(path_base + ".json", "w") as f:
        json.dump(meta, f, indent=2)


def main():
    args = parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    body_obj = bpy.data.objects[args.body]
    body = BodySurface(body_obj)
    flat = [i for t in body.tris for i in t]
    with open(os.path.join(args.out_dir, "body.tris.bin"), "wb") as f:
        f.write(struct.pack("<%dI" % len(flat), *flat))
    for garment in bpy.data.collections[args.collection].objects:
        if garment.type != "MESH":
            continue
        arrays, n_islands = bind_garment(garment, body)
        meta = {"garment": garment.name, "body": body_obj.name,
                "body_vertex_count": len(body.verts), "body_triangle_count": len(body.tris),
                "vertex_count": len(garment.data.vertices), "rigid_islands": n_islands,
                "bone_names": body.group_names}
        write(os.path.join(args.out_dir, garment.name + ".bind"), arrays, meta)
        print(f"BIND {garment.name}: {meta['vertex_count']} verts, {n_islands} rigid islands")


if __name__ == "__main__":
    main()
