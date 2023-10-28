const std = @import("std");
const ethsecp = @import("secp256k1.zig");

pub fn main() void {
    _ = ethsecp.Secp256k1.init();
}
