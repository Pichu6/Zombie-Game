"""A minimal reader for R's .rds files (XDR serialization, version 2 or 3, gzip-compressed): enough for data frames
of numbers, strings and factors. Returns plain Python values; a data frame becomes {column: list}.
Used by tools/data/global_tables.py for the Wittgenstein Centre's files (spec 46); no third-party package needed."""
import gzip
import struct

NILSXP, SYMSXP, LISTSXP, CLOSXP, ENVSXP, PROMSXP, LANGSXP = 0, 1, 2, 3, 4, 5, 6
CHARSXP, LGLSXP, INTSXP, REALSXP, CPLXSXP, STRSXP, VECSXP = 9, 10, 13, 14, 15, 16, 19
REFSXP, NILVALUE_SXP, GLOBALENV_SXP, EMPTYENV_SXP, BASEENV_SXP, ALTREP_SXP = 255, 254, 253, 242, 241, 238
NA_INT = -2147483648


class Reader:
    def __init__(self, data):
        self.b, self.p, self.refs = data, 0, []

    def int(self):
        v = struct.unpack_from(">i", self.b, self.p)[0]
        self.p += 4
        return v

    def double(self, n):
        v = struct.unpack_from(f">{n}d", self.b, self.p)
        self.p += 8 * n
        return list(v)

    def ints(self, n):
        v = struct.unpack_from(f">{n}i", self.b, self.p)
        self.p += 4 * n
        return [None if x == NA_INT else x for x in v]

    def length(self):
        n = self.int()
        if n == -1:
            hi, lo = self.int(), self.int()
            n = (hi << 32) + lo
        return n

    def item(self):
        flags = self.int()
        t = flags & 0xFF
        has_attr, has_tag = bool(flags & (1 << 9)), bool(flags & (1 << 10))
        if t in (NILVALUE_SXP, GLOBALENV_SXP, EMPTYENV_SXP, BASEENV_SXP):
            return None
        if t == REFSXP:
            return self.refs[(flags >> 8) - 1]
        if t == SYMSXP:
            name = self.item()
            self.refs.append(name)
            return name
        if t == ENVSXP:
            env = {}
            self.refs.append(env)
            self.int()  # locked
            for _ in range(4):  # enclos, frame, hashtab, attrib
                self.item()
            return env
        if t in (LISTSXP, LANGSXP):  # pairlist: attributes, tag, car, cdr
            out = {}
            node_attr = self.item() if has_attr else None
            del node_attr
            tag = self.item() if has_tag else None
            out[tag] = self.item()
            rest = self.item()
            if isinstance(rest, dict):
                out.update(rest)
            return out
        if t == CHARSXP:
            n = self.int()
            if n == -1:
                return None
            s = self.b[self.p:self.p + n].decode("utf-8", "replace")
            self.p += n
            return s
        if t == ALTREP_SXP:  # compact sequences and wrappers: info, state, attributes
            info, state, attr = self.item(), self.item(), self.item()
            cls = info.get(None) if isinstance(info, dict) else info
            if isinstance(state, list) and cls == "compact_intseq":
                n, start, step = int(state[0]), int(state[1]), int(state[2])
                return [start + i * step for i in range(n)]
            if isinstance(state, dict):  # wrap_*: the wrapped vector comes first
                return next(iter(state.values()))
            return state
        if t in (LGLSXP, INTSXP):
            v = self.ints(self.length())
        elif t == REALSXP:
            v = self.double(self.length())
        elif t == STRSXP:
            v = [self.item() for _ in range(self.length())]
        elif t == VECSXP:
            v = [self.item() for _ in range(self.length())]
        else:
            raise ValueError(f"rds: unsupported type {t}")
        if has_attr:
            attrs = self.item()
            levels = attrs.get("levels") if isinstance(attrs, dict) else None
            names = attrs.get("names") if isinstance(attrs, dict) else None
            if levels is not None and t == INTSXP:  # a factor
                v = [None if x is None else levels[x - 1] for x in v]
            elif names is not None and t == VECSXP:  # a list or a data frame
                v = dict(zip(names, v))
        return v


def read_rds(path):
    data = gzip.open(path, "rb").read()
    if data[:2] != b"X\n":
        raise ValueError("rds: only the XDR format is supported")
    r = Reader(data)
    r.p = 2
    version = r.int()
    r.int()  # writer R version
    r.int()  # minimal reader version
    if version == 3:
        r.p += 4 + r.int()  # native encoding
    return r.item()
