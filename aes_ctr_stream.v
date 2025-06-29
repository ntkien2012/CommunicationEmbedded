module aes_ctr_stream #(
    parameter CTR_WIDTH = 32  // width of the counter in LSBs
)(
    input           clk,
    input           rst,         // active-high reset
    input           start,       // pulse to start key-expansion + session
    input  [255:0]  key,         // AES-256 key
    input  [127:0]  iv_init,     // 96-bit nonce || initial counter
    input           valid_in,    // upstream valid
    input           last_block,  // indicates final block
    input  [3:0]    last_len,    // number of valid bytes in final block (1..16)
    input  [127:0]  data_in,     // plaintext/ciphertext input (zero-padded)
    output          ready_in,    // wrapper ready to accept data_in
    output [127:0]  data_out,    // keystream ^ data_in, masked
    output          valid_out,   // wrapper valid output
    input           ready_out,   // downstream ready
    output reg      done         // one-cycle when last block done
);

    // Internal registers/wires
    reg  [127:0] counter_block;
    reg          processing;
    reg          done_pulse;

    wire [127:0] keystream_block;
    wire         aes_valid_out;
    wire         key_exp_done;

    // Instantiate AES core with key_exp_done exposed
    AES256_PipelineTop u_aes (
        .clk         (clk),
        .rst         (rst),
        .start       (start),
        .valid_in    (processing && key_exp_done && valid_in),
        .plaintext   (counter_block),
        .key         (key),
        .ready_out   (ready_out),
        .valid_out   (aes_valid_out),
        .ciphertext  (keystream_block),
        .key_exp_done(key_exp_done),
        .done        ()
    );

    // Flow control
    assign ready_in  = processing && key_exp_done && ready_out;
    assign valid_out = processing && aes_valid_out;

    // Full XOR output
    wire [127:0] full_out = keystream_block ^ data_in;

    // Masked output: only first last_len bytes from MSB side on final block
    reg [127:0] masked_out;
    integer i;
    always @(*) begin
        if (processing && last_block) begin
            masked_out = 128'd0;
            // Copy top bytes: byte index 0 at bits [127:120]
            for (i = 0; i < last_len; i = i + 1) begin
                masked_out[127 - i*8 -: 8] = full_out[127 - i*8 -: 8];
            end
        end else begin
            masked_out = full_out;
        end
    end

    assign data_out = masked_out;

    // FSM: counter increment & done pulse
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            counter_block <= 128'd0;
            processing    <= 1'b0;
            done_pulse    <= 1'b0;
        end else begin
            done_pulse <= 1'b0;

            if (start && !processing) begin
                counter_block <= iv_init;
                processing    <= 1'b1;
            end else if (processing && aes_valid_out && ready_out) begin
                if (last_block) begin
                    processing <= 1'b0;
                    done_pulse <= 1'b1;
                end else begin
                    counter_block <= {
                        counter_block[127:CTR_WIDTH],
                        counter_block[CTR_WIDTH-1:0] + 1'b1
                    };
                end
            end
        end
    end

    // Latch done
    always @(posedge clk or posedge rst) begin
        if (rst) done <= 1'b0;
        else      done <= done_pulse;
    end

endmodule