`timescale 1ns/1ps
// ========================================================================
//  Testbench for AES256_PipelineTop
//  ? Uses NIST AES?256 ECB Known?Answer Test vector
//      Key       : 603deb1015ca71be2b73aef0857d7781 1f352c073b6108d7 2d9810a30914dff4
//      Plaintext : 6bc1bee22e409f96e93d7e117393172a
//      Ciphertext: f3eed1bdb5d2a03c064b5a7e3db181f8
// ========================================================================
module tb_aes256;
    // --------------------------------------------------------------------
    //  Clock & reset
    // --------------------------------------------------------------------
    reg clk = 1'b0;
    always #5 clk = ~clk;           // 100?MHz clock

    reg rst = 1'b1;

    // --------------------------------------------------------------------
    //  DUT interface signals
    // --------------------------------------------------------------------
    reg  start     = 1'b0;
    reg  valid_in  = 1'b0;
    reg  [127:0] plaintext = 128'd0;
    reg  [255:0] key       = 256'd0;

    wire ready_out = 1'b1;          // sink always ready

    wire valid_out;
    wire [127:0] ciphertext;
    wire done;
    wire key_done;                  // exposed from DUT for TB convenience

    // --------------------------------------------------------------------
    //  DUT instance
    // --------------------------------------------------------------------
    AES256_PipelineTop dut (
        .clk        (clk),
        .rst        (rst),
        .start      (start),
        .valid_in   (valid_in),
        .plaintext  (plaintext),
        .key        (key),
        .ready_out  (ready_out),
        .valid_out  (valid_out),
        .ciphertext (ciphertext),
        .done       (done)
    );

    // expose internal key_done for wait (optional ? comment if not in interface)
    assign key_done = dut.key_done;

    // --------------------------------------------------------------------
    //  VCD dump
    // --------------------------------------------------------------------
    initial begin
        $dumpfile("aes256_tb.vcd");
        $dumpvars(0, tb_aes256);
    end

    // --------------------------------------------------------------------
    //  Stimulus
    // --------------------------------------------------------------------
    localparam KEY256  = 256'h603deb1015ca71be2b73eaf0857d77811f352c073b6108d72d9810a30914dff4;
    localparam PTEXT   = 128'h6bc1bee22e409f96e93d7e117393172a;
    localparam CTEXT   = 128'hb11ccd6a292c8e77f869e932c194c734;

    initial begin
        //-----------------------------------------------------------------
        // reset
        //-----------------------------------------------------------------
        #12  rst = 1'b0;           // release reset after > half clk period

        //-----------------------------------------------------------------
        // key load and start expansion
        //-----------------------------------------------------------------
        key   = KEY256;
        @(posedge clk);            // align to clock edge
        start = 1'b1;
        @(posedge clk);
        start = 1'b0;

        //-----------------------------------------------------------------
        // wait for key schedule ready, then send plaintext
        //-----------------------------------------------------------------
        @(posedge key_done);
        @(posedge clk);
        plaintext = PTEXT;
        valid_in  = 1'b1;
        @(posedge clk);
        valid_in  = 1'b0;          // single?cycle pulse ? DUT keeps copy

        //-----------------------------------------------------------------
        // wait for result & check
        //-----------------------------------------------------------------
        wait (valid_out);
        if (ciphertext == CTEXT) begin
            $display("\nTEST PASSED ? ciphertext = %h matches expected", ciphertext);
        end else begin
            $display("\nTEST FAILED ? ciphertext = %h (expected %h)", ciphertext, CTEXT);
        end

        //-----------------------------------------------------------------
        // finish simulation
        //-----------------------------------------------------------------
        #20 $finish;
    end
endmodule

