module aes_ctr_top (
    input         clk,
    input         rst,

    // Input plaintext byte stream
    input  [7:0]  data_byte_in,
    input         valid_byte_in,
    input         data_done_in,
    output        ready_byte_up,

    // Partial length output (for downstream)
    output [6:0] partial_len_out,

    // AES configuration
    input  [255:0] key,
    input  [127:0] iv_init,
    input         start_ctr,       

    // Output ciphertext
    output [127:0] data_out,
    output        valid_out,
    input         ready_out,
    output        done          
);

    // Internal signals
    wire [127:0] data_block;
    wire         valid_block;
    wire         ready_block;
    wire         last_block;
    wire [6:0]   partial_len;

    wire [127:0] ctr_data_out;
    wire         ctr_valid_out;
    wire         ctr_done;

    // 1. Byte FIFO + Splitter
    splitter_with_fifo u_splitter (
        .clk(clk),
        .rst(rst),
        .data_byte_in(data_byte_in),
        .valid_byte_in(valid_byte_in),
        .data_done_in(data_done_in),
        .ready_byte_up(ready_byte_up),

        .data_block(data_block),
        .valid_block(valid_block),
        .ready_block(ready_block),
        .last_block(last_block),
        .partial_len(partial_len)
    );

    assign partial_len_out = partial_len;

    // 2. AES-CTR Stream Module
    aes_ctr_stream u_ctr (
        .clk(clk),
        .rst(rst),
        .start(start_ctr),
        .key(key),
        .iv_init(iv_init),

        .valid_in(valid_block),
        .last_block(last_block),
        .data_in(data_block),
        .ready_in(ready_block),

        .data_out(ctr_data_out),
        .valid_out(ctr_valid_out),
        .ready_out(ready_out),
        .done(ctr_done)
    );

    // Output passthrough v?i masking
    reg [127:0] masked_out;
    reg [15:0]  byte_mask;
    integer i;

    always @(*) begin
        masked_out = ctr_data_out;
        byte_mask  = 16'hFFFF;
        if (last_block && partial_len != 0) begin
            case (partial_len)
                7'd8:   byte_mask = 16'h0001;
                7'd16:  byte_mask = 16'h0003;
                7'd24:  byte_mask = 16'h0007;
                7'd32:  byte_mask = 16'h000F;
                7'd40:  byte_mask = 16'h001F;
                7'd48:  byte_mask = 16'h003F;
                7'd56:  byte_mask = 16'h007F;
                7'd64:  byte_mask = 16'h00FF;
                7'd72:  byte_mask = 16'h01FF;
                7'd80:  byte_mask = 16'h03FF;
                7'd88:  byte_mask = 16'h07FF;
                7'd96:  byte_mask = 16'h0FFF;
                7'd104: byte_mask = 16'h1FFF;
                7'd112: byte_mask = 16'h3FFF;
                7'd120: byte_mask = 16'h7FFF;
                7'd128: byte_mask = 16'hFFFF;
                default: byte_mask = 16'hFFFF;
            endcase
            for (i = 0; i < 16; i = i + 1) begin
                if (!byte_mask[i])
                    masked_out[i*8 +: 8] = 8'd0;
            end
        end
    end

    assign data_out  = masked_out;
    assign valid_out = ctr_valid_out;
    assign done      = ctr_done;

endmodule

