// AES-256 MODULES - Pipelined Design Version with reliable valid/ready control
`timescale 1ns/1ps

`include "SBoxROM.v"

// ============================
// 1. AddRoundKey (1 cycle)
// ============================
module AddRoundKey(
    clk,
    rst,
    valid_in,
    ready_in,
    state_in,
    round_key,
    state_out,
    valid_out,
    ready_out
);

input        clk;
input        rst;
input        valid_in;
input        ready_in;
input  [127:0] state_in;
input  [127:0] round_key;

output [127:0] state_out;
output         valid_out;
output         ready_out;

reg [127:0] state_reg;
reg         valid_reg;

assign state_out = state_reg;
assign valid_out = valid_reg;
assign ready_out = !valid_reg || ready_in;

always @(posedge clk or posedge rst) begin
    if (rst) begin
        valid_reg <= 1'b0;
    end else if (valid_reg && ready_in) begin
        valid_reg <= 1'b0;
    end else if (!valid_reg && valid_in) begin
        state_reg <= state_in ^ round_key;
        valid_reg <= 1'b1;
    end
end

endmodule



// ============================
// 2. SubBytes (1 cycle)
// ============================
module SubBytes(
    clk,
    rst,
    valid_in,
    ready_in,
    state_in,
    state_out,
    valid_out,
    ready_out
);

input         clk;
input         rst;
input         valid_in;
input         ready_in;
input  [127:0] state_in;

output [127:0] state_out;
output         valid_out;
output         ready_out;

reg    [127:0] state_reg;
reg            valid_reg;
wire           ready_out;
wire   [127:0] state_out;
wire           valid_out;

assign state_out = state_reg;
assign valid_out = valid_reg;
assign ready_out = !valid_reg || ready_in;

// Wires for each S-box output byte
wire [7:0] sbox_out [15:0];

// Instantiate 16 S-boxes, one for each byte of state_in
SBoxROM u_sbox0  (.in_byte(state_in[127:120]), .out_byte(sbox_out[0]));
SBoxROM u_sbox1  (.in_byte(state_in[119:112]), .out_byte(sbox_out[1]));
SBoxROM u_sbox2  (.in_byte(state_in[111:104]), .out_byte(sbox_out[2]));
SBoxROM u_sbox3  (.in_byte(state_in[103:96]),  .out_byte(sbox_out[3]));
SBoxROM u_sbox4  (.in_byte(state_in[95:88]),   .out_byte(sbox_out[4]));
SBoxROM u_sbox5  (.in_byte(state_in[87:80]),   .out_byte(sbox_out[5]));
SBoxROM u_sbox6  (.in_byte(state_in[79:72]),   .out_byte(sbox_out[6]));
SBoxROM u_sbox7  (.in_byte(state_in[71:64]),   .out_byte(sbox_out[7]));
SBoxROM u_sbox8  (.in_byte(state_in[63:56]),   .out_byte(sbox_out[8]));
SBoxROM u_sbox9  (.in_byte(state_in[55:48]),   .out_byte(sbox_out[9]));
SBoxROM u_sbox10 (.in_byte(state_in[47:40]),   .out_byte(sbox_out[10]));
SBoxROM u_sbox11 (.in_byte(state_in[39:32]),   .out_byte(sbox_out[11]));
SBoxROM u_sbox12 (.in_byte(state_in[31:24]),   .out_byte(sbox_out[12]));
SBoxROM u_sbox13 (.in_byte(state_in[23:16]),   .out_byte(sbox_out[13]));
SBoxROM u_sbox14 (.in_byte(state_in[15:8]),    .out_byte(sbox_out[14]));
SBoxROM u_sbox15 (.in_byte(state_in[7:0]),     .out_byte(sbox_out[15]));

integer j;

always @(posedge clk or posedge rst) begin
    if (rst) begin
        valid_reg <= 1'b0;
    end else if (valid_reg && ready_in) begin
        valid_reg <= 1'b0;
    end else if (!valid_reg && valid_in) begin
        state_reg[127:120] <= sbox_out[0];
        state_reg[119:112] <= sbox_out[1];
        state_reg[111:104] <= sbox_out[2];
        state_reg[103:96]  <= sbox_out[3];
        state_reg[95:88]   <= sbox_out[4];
        state_reg[87:80]   <= sbox_out[5];
        state_reg[79:72]   <= sbox_out[6];
        state_reg[71:64]   <= sbox_out[7];
        state_reg[63:56]   <= sbox_out[8];
        state_reg[55:48]   <= sbox_out[9];
        state_reg[47:40]   <= sbox_out[10];
        state_reg[39:32]   <= sbox_out[11];
        state_reg[31:24]   <= sbox_out[12];
        state_reg[23:16]   <= sbox_out[13];
        state_reg[15:8]    <= sbox_out[14];
        state_reg[7:0]     <= sbox_out[15];
        valid_reg <= 1'b1;
    end
end

endmodule




// ============================
// 3. ShiftRows (1 cycle)
// ============================
module ShiftRows(
    clk,
    rst,
    valid_in,
    ready_in,
    state_in,
    state_out,
    valid_out,
    ready_out
);

input         clk;
input         rst;
input         valid_in;
input         ready_in;
input  [127:0] state_in;

output [127:0] state_out;
output         valid_out;
output         ready_out;

reg    [127:0] state_reg;
reg            valid_reg;

assign state_out = state_reg;
assign valid_out = valid_reg;
assign ready_out = !valid_reg || ready_in;

always @(posedge clk or posedge rst) begin
    if (rst) begin
        valid_reg <= 1'b0;
    end else if (valid_reg && ready_in) begin
        valid_reg <= 1'b0;
    end else if (!valid_reg && valid_in) begin
        state_reg <= {
            // Row 0: no shift
            state_in[127:120], state_in[87:80],  state_in[47:40],  state_in[7:0],
            // Row 1: shift left by 1 byte
            state_in[95:88],   state_in[55:48],  state_in[15:8],   state_in[103:96],
            // Row 2: shift left by 2 bytes
            state_in[63:56],   state_in[23:16],  state_in[111:104],state_in[71:64],
            // Row 3: shift left by 3 bytes
            state_in[31:24],   state_in[119:112],state_in[79:72],  state_in[39:32]
        };
        valid_reg <= 1'b1;
    end
end

endmodule



// ============================
// 4. MixColumns (1 cycle)
// ============================
module MixColumns(
    clk,
    rst,
    valid_in,
    ready_in,
    state_in,
    state_out,
    valid_out,
    ready_out
);

input         clk;
input         rst;
input         valid_in;
input         ready_in;
input  [127:0] state_in;

output [127:0] state_out;
output         valid_out;
output         ready_out;

reg    [127:0] state_reg;
reg            valid_reg;

assign state_out = state_reg;
assign valid_out = valid_reg;
assign ready_out = !valid_reg || ready_in;

// GF(2^8) multiply by 2
function [7:0] mul2;
    input [7:0] b;
    begin
        mul2 = {b[6:0],1'b0} ^ (8'h1b & {8{b[7]}});
    end
endfunction

// GF(2^8) multiply by 3 = mul2(b) ^ b
function [7:0] mul3;
    input [7:0] b;
    begin
        mul3 = mul2(b) ^ b;
    end
endfunction

// Mix one column (32 bits = 4 bytes)
function [31:0] mix_column;
    input [31:0] w;
    reg [7:0] b0, b1, b2, b3;
    begin
        b0 = w[31:24];
        b1 = w[23:16];
        b2 = w[15:8];
        b3 = w[7:0];

        mix_column[31:24] = mul2(b0) ^ mul3(b1) ^ b2        ^ b3;
        mix_column[23:16] = b0        ^ mul2(b1) ^ mul3(b2) ^ b3;
        mix_column[15:8]  = b0        ^ b1        ^ mul2(b2) ^ mul3(b3);
        mix_column[7:0]   = mul3(b0) ^ b1        ^ b2        ^ mul2(b3);
    end
endfunction

always @(posedge clk or posedge rst) begin
    if (rst) begin
        valid_reg <= 1'b0;
    end else if (valid_reg && ready_in) begin
        valid_reg <= 1'b0;
    end else if (!valid_reg && valid_in) begin
        state_reg[127:96] <= mix_column(state_in[127:96]);
        state_reg[95:64]  <= mix_column(state_in[95:64]);
        state_reg[63:32]  <= mix_column(state_in[63:32]);
        state_reg[31:0]   <= mix_column(state_in[31:0]);
        valid_reg <= 1'b1;
    end
end

endmodule



// ============================
// 5. KeyExpansion256 (AES-256 key schedule)
// ============================
module KeyExpansion256(
    clk,
    rst,
    start,
    key_in,
    round_keys,
    done
);

input         clk;
input         rst;
input         start;
input  [255:0] key_in;

output [1919:0] round_keys;  // 15 * 128 = 1919:0
output          done;

reg [31:0] w [0:59];
reg [5:0]  idx;
reg        busy;
reg        key_expansion_done;  // Added: Register to track completion

// Done is asserted when key expansion is complete and stays high until next start
assign done = key_expansion_done;

// Rcon
reg [7:0] Rcon [0:13];
initial begin
    Rcon[0]  = 8'h01; Rcon[1]  = 8'h02; Rcon[2]  = 8'h04; Rcon[3]  = 8'h08;
    Rcon[4]  = 8'h10; Rcon[5]  = 8'h20; Rcon[6]  = 8'h40; Rcon[7]  = 8'h80;
    Rcon[8]  = 8'h1B; Rcon[9]  = 8'h36; Rcon[10] = 8'h6C; Rcon[11] = 8'hD8;
    Rcon[12] = 8'hAB; Rcon[13] = 8'h4D;
end

// SBoxROM
reg  [7:0] sbox_in;
wire [7:0] sbox_out;
SBoxROM u_sbox (
    .in_byte(sbox_in),
    .out_byte(sbox_out)
);

// FSM for SubWord
reg [1:0]  byte_idx;
reg [31:0] tmp;
reg [7:0]  subword_bytes [0:3];
reg        do_subword;
reg        subword_done;
reg        apply_rcon;

// Output round keys packed
reg [1919:0] round_keys;
integer r;

always @(posedge clk or posedge rst) begin
    if (rst) begin
        idx                <= 0;
        busy              <= 0;
        byte_idx          <= 0;
        do_subword        <= 0;
        subword_done      <= 0;
        key_expansion_done <= 0;
    end else if (start && !busy) begin
        for (r = 0; r < 8; r = r + 1) begin
            w[r] <= key_in[255 - r*32 -: 32];
        end
        idx                <= 8;
        busy              <= 1;
        do_subword        <= 0;
        key_expansion_done <= 0;
    end else if (busy && idx < 60) begin
        if (!do_subword && !subword_done) begin
            tmp = w[idx - 1];

            if (idx % 8 == 0) begin
                tmp = {tmp[23:0], tmp[31:24]}; // RotWord
                apply_rcon <= 1;
                do_subword <= 1;
                byte_idx   <= 0;
                sbox_in    <= tmp[31:24];
            end else if (idx % 8 == 4) begin
                apply_rcon <= 0;
                do_subword <= 1;
                byte_idx   <= 0;
                sbox_in    <= tmp[31:24];
            end else begin
                w[idx] <= w[idx - 8] ^ tmp;
                idx    <= idx + 1;
                if (idx == 59) begin
                    busy <= 0;
                    key_expansion_done <= 1;  // Set done when last key is generated
                end
            end

        end else if (do_subword) begin
            subword_bytes[byte_idx] <= sbox_out;
            byte_idx <= byte_idx + 1;

            case (byte_idx)
                2'd0: sbox_in <= tmp[23:16];
                2'd1: sbox_in <= tmp[15:8];
                2'd2: sbox_in <= tmp[7:0];
                2'd3: begin
                    tmp = {subword_bytes[0], subword_bytes[1], subword_bytes[2], sbox_out};
                    if (apply_rcon)
                        tmp = tmp ^ {Rcon[idx/8 - 1], 24'h0};
                    w[idx] <= w[idx - 8] ^ tmp;
                    idx    <= idx + 1;
                    if (idx == 59) begin
                        busy <= 0;
                        key_expansion_done <= 1;  // Set done when last key is generated
                    end
                    do_subword   <= 0;
                    subword_done <= 1;
                end
            endcase

        end else if (subword_done) begin
            subword_done <= 0;
        end
    end
end

// Pack round keys to flat output
always @(w[0] or w[1] or w[2] or w[3] or
         w[4] or w[5] or w[6] or w[7] or
         w[8] or w[9] or w[10] or w[11] or
         w[12] or w[13] or w[14] or w[15] or
         w[16] or w[17] or w[18] or w[19] or
         w[20] or w[21] or w[22] or w[23] or
         w[24] or w[25] or w[26] or w[27] or
         w[28] or w[29] or w[30] or w[31] or
         w[32] or w[33] or w[34] or w[35] or
         w[36] or w[37] or w[38] or w[39] or
         w[40] or w[41] or w[42] or w[43] or
         w[44] or w[45] or w[46] or w[47] or
         w[48] or w[49] or w[50] or w[51] or
         w[52] or w[53] or w[54] or w[55] or
         w[56] or w[57] or w[58] or w[59])
begin
    for (r = 0; r < 15; r = r + 1) begin
        round_keys[r*128 +: 128] =
            {w[r*4], w[r*4 + 1], w[r*4 + 2], w[r*4 + 3]};
    end
end

endmodule





// ============================
// 6. AES256_PipelineTop with KeyExpansion256
// ============================
module AES256_PipelineTop (
    input  wire         clk,
    input  wire         rst,
    input  wire         start,

    // data / key input
    input  wire         valid_in,
    input  wire [127:0] plaintext,
    input  wire [255:0] key,

    // sink interface
    input  wire         ready_out,

    // cipher text output
    output reg          valid_out,
    output reg  [127:0] ciphertext,
    output wire         done,

    // key expansion done
    output wire         key_exp_done
);

// ----------------------------------------------------------------
//  1) round?key generation (KeyExpansion256)
// ----------------------------------------------------------------
wire [1919:0] round_keys_flat;       // 15 * 128 = 1919:0
wire          key_done;

KeyExpansion256 u_keyexp (
    .clk        (clk),
    .rst        (rst),
    .start      (start),
    .key_in     (key),
    .round_keys (round_keys_flat),
    .done       (key_exp_done)
);

// ----------------------------------------------------------------
//  2) Pipeline state arrays
// ----------------------------------------------------------------
wire [127:0] state [0:14];
wire         valid [0:14];

// ready chain from sink backwards
wire ready_internal [0:15];
assign ready_internal[15] = ready_out;            // sink

genvar gi;
for (gi = 14; gi >= 0; gi = gi - 1) begin : READY_CHAIN
    assign ready_internal[gi] = ready_out;  // Simplified: all stages ready when sink is ready
end

// ----------------------------------------------------------------
//  3) Round 0 (initial AddRoundKey)
// ----------------------------------------------------------------
AddRoundKey u_round0 (
    .clk       (clk),
    .rst       (rst),
    .valid_in  (valid_in && key_exp_done && ready_out),  // Use ready_out directly
    .ready_in  (ready_out),                          // Use ready_out directly
    .state_in  (plaintext),
    .round_key (round_keys_flat[ 0 +: 128 ]), // k0
    .state_out (state[0]),
    .valid_out (valid[0]),
    .ready_out ()
);

// ----------------------------------------------------------------
//  4) Rounds 1 ? 13 (generate loop)
// ----------------------------------------------------------------
wire [127:0] sb_out [1:13], sr_out [1:13], mc_out [1:13];
wire         sb_valid [1:13], sr_valid [1:13], mc_valid [1:13];
wire         sb_ready [1:13], sr_ready [1:13], mc_ready [1:13];

generate
    for (gi = 1; gi <= 13; gi = gi + 1) begin : ROUND_GEN
        //----------------------------------------------------------
        // ready chain inside round gi
        //----------------------------------------------------------
        assign mc_ready[gi] = !mc_valid[gi] || ready_internal[gi+1];
        assign sr_ready[gi] = !sr_valid[gi] || mc_ready[gi];
        assign sb_ready[gi] = !sb_valid[gi] || sr_ready[gi];

        //----------------------------------------------------------
        //  SubBytes
        //----------------------------------------------------------
        SubBytes u_sb (
            .clk       (clk), .rst(rst),
            .valid_in  (valid[gi-1]),
            .ready_in  (sb_ready[gi]),
            .state_in  (state[gi-1]),
            .state_out (sb_out[gi]),
            .valid_out (sb_valid[gi]),
            .ready_out ()
        );

        //----------------------------------------------------------
        //  ShiftRows
        //----------------------------------------------------------
        ShiftRows u_sr (
            .clk       (clk), .rst(rst),
            .valid_in  (sb_valid[gi]),
            .ready_in  (sr_ready[gi]),
            .state_in  (sb_out[gi]),
            .state_out (sr_out[gi]),
            .valid_out (sr_valid[gi]),
            .ready_out ()
        );

        //----------------------------------------------------------
        //  MixColumns
        //----------------------------------------------------------
        MixColumns u_mc (
            .clk       (clk), .rst(rst),
            .valid_in  (sr_valid[gi]),
            .ready_in  (mc_ready[gi]),
            .state_in  (sr_out[gi]),
            .state_out (mc_out[gi]),
            .valid_out (mc_valid[gi]),
            .ready_out ()
        );

        //----------------------------------------------------------
        //  AddRoundKey ? output of round gi
        //----------------------------------------------------------
        AddRoundKey u_ark (
            .clk       (clk), .rst(rst),
            .valid_in  (mc_valid[gi]),
            .ready_in  (ready_internal[gi+1]),
            .state_in  (mc_out[gi]),
            .round_key (round_keys_flat[ gi*128 +: 128 ]),
            .state_out (state[gi]),
            .valid_out (valid[gi]),
            .ready_out ()
        );
    end
endgenerate

// ----------------------------------------------------------------
//  5) Final round 14 (SubBytes ? ShiftRows ? AddRoundKey)
// ----------------------------------------------------------------
wire sb14_valid, sr14_valid;
wire [127:0] sb14_out, sr14_out, final_out;
wire sb14_ready, sr14_ready;

assign sr14_ready = !sr14_valid || ready_out;
assign sb14_ready = !sb14_valid || sr14_ready;

// 14a) SubBytes
SubBytes u_sb14 (
    .clk       (clk), .rst(rst),
    .valid_in  (valid[13]),
    .ready_in  (sb14_ready),
    .state_in  (state[13]),
    .state_out (sb14_out),
    .valid_out (sb14_valid),
    .ready_out ()
);

// 14b) ShiftRows
ShiftRows u_sr14 (
    .clk       (clk), .rst(rst),
    .valid_in  (sb14_valid),
    .ready_in  (sr14_ready),
    .state_in  (sb14_out),
    .state_out (sr14_out),
    .valid_out (sr14_valid),
    .ready_out ()
);

// 14c) AddRoundKey
AddRoundKey u_final (
    .clk       (clk), .rst(rst),
    .valid_in  (sr14_valid),
    .ready_in  (ready_out),
    .state_in  (sr14_out),
    .round_key (round_keys_flat[ 14*128 +: 128 ]),
    .state_out (final_out),
    .valid_out (final_valid),
    .ready_out ()
);

// ----------------------------------------------------------------
//  6) Output & done flag
// ----------------------------------------------------------------
reg encrypt_done;

always @(posedge clk or posedge rst) begin
    if (rst) begin
        valid_out    <= 1'b0;
        ciphertext   <= 128'd0;
        encrypt_done <= 1'b0;
    end else begin
        if (final_valid && ready_out) begin
            ciphertext   <= final_out;
            valid_out    <= 1'b1;
            encrypt_done <= 1'b1;
        end else if (valid_out && ready_out) begin
            valid_out <= 1'b0;
        end
    end
end

assign done = encrypt_done;

endmodule



