"""CPU (numpy) reference of the engine's "wrap" pass.

This is the executable spec of the compute shader: same maths, one garment
vertex per thread. Input: the deformed body (morphs + skeleton) and the binding
exported by export_garment_binding.py. Output: garment positions.
"""
import json

import numpy as np


def load_binding(base):
    meta = json.load(open(base + ".json"))
    raw = open(base + ".bin", "rb").read()
    out, pos = {}, 0
    for item in meta["layout"]:
        dt = {"u32": "<u4", "f32": "<f4", "u16": "<u2"}[item["type"]]
        arr = np.frombuffer(raw, dtype=dt, count=item["count"], offset=pos)
        pos += arr.nbytes
        out[item["name"]] = arr
    n = meta["vertex_count"]
    out["bary"] = out["bary"].reshape(n, 2)
    out["offset_tbn"] = out["offset_tbn"].reshape(n, 3)
    out["body_uv"] = out["body_uv"].reshape(n, 2)
    out["bone_index"] = out["bone_index"].reshape(n, 4)
    out["bone_weight"] = out["bone_weight"].reshape(n, 4)
    return meta, out


def vertex_normals(verts, tris):
    a, b, c = verts[tris[:, 0]], verts[tris[:, 1]], verts[tris[:, 2]]
    face = np.cross(b - a, c - a)  # area-weighted, same as the exporter
    acc = np.zeros_like(verts)
    for k in range(3):
        np.add.at(acc, tris[:, k], face)
    return acc / np.linalg.norm(acc, axis=1, keepdims=True)


def wrap(body_verts, body_tris, bind, skinned_only=None, min_gap=0.004):
    """body_verts: (V,3) body deformed this frame. body_tris: (T,3).
    skinned_only: (N,3) garment deformed by bones only (for follow<1), optional."""
    normals = vertex_normals(body_verts, body_tris)
    tri = body_tris[bind["tri_id"]]
    u, v = bind["bary"][:, :1], bind["bary"][:, 1:]
    w = 1.0 - u - v
    a, b, c = body_verts[tri[:, 0]], body_verts[tri[:, 1]], body_verts[tri[:, 2]]
    p = a * w + b * u + c * v
    n = normals[tri[:, 0]] * w + normals[tri[:, 1]] * u + normals[tri[:, 2]] * v
    n /= np.linalg.norm(n, axis=1, keepdims=True)
    e = b - a
    t = e - n * np.sum(e * n, axis=1, keepdims=True)
    t /= np.linalg.norm(t, axis=1, keepdims=True)
    bt = np.cross(n, t)
    off = bind["offset_tbn"].copy()
    rigid = bind["island"] != 0xFFFFFFFF
    # Minimum gap: cloth never goes below min_gap above its anchor point.
    off[~rigid, 2] = np.maximum(off[~rigid, 2], min_gap)
    wrapped = p + t * off[:, :1] + bt * off[:, 1:2] + n * off[:, 2:3]
    if skinned_only is None:
        return wrapped
    f = bind["follow"][:, None]
    return skinned_only + f * (wrapped - skinned_only)
