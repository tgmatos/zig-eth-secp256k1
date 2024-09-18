const std = @import("std");
const ethsecp = @import("secp256k1.zig");

pub fn main() !void {
    const sec = try ethsecp.Secp256k1.init();
    const keypair = try sec.generate_keypair();
    std.debug.print("{s}", .{keypair.pubkey});
}

test "secp" {
    _ = @import("secp256k1.zig");
}
