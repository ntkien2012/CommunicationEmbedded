`timescale 1ns/1ps

module tb_aes_ctr_stream;

    localparam CTR_WIDTH = 32;
    integer     i;

    // Clock & reset
    reg         clk;
    reg         rst_n;

    // DUT inputs
    reg         start;
    reg [255:0] key;
    reg [127:0] iv_init;
    reg         valid_in;
    reg         last_block;
    reg  [3:0]  last_len;
    reg [127:0] data_in;
    reg         ready_out;

    // DUT outputs
    wire        ready_in;
    wire        valid_out;
    wire [127:0] data_out;
    wire        done;

    // Instantiate DUT (named mapping)
    aes_ctr_stream #(.CTR_WIDTH(CTR_WIDTH)) dut (
        .clk        (clk),
        .rst        (~rst_n),
        .start      (start),
        .key        (key),
        .iv_init    (iv_init),
        .valid_in   (valid_in),
        .last_block (last_block),
        .last_len   (last_len),
        .data_in    (data_in),
        .ready_in   (ready_in),
        .data_out   (data_out),
        .valid_out  (valid_out),
        .ready_out  (ready_out),
        .done       (done)
    );

    // Clock gen: 10ns period
    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    // Monitor
    always @(posedge clk) begin
        $display("%0t | rst=%b start=%b ready_in=%b valid_out=%b done=%b data_out[127:16]=%h",
                 $time, rst_n, start, ready_in, valid_out, done, data_out[127:16]);
    end

    initial begin
        // Init
        rst_n      = 0;
        start      = 0;
        valid_in   = 0;
        last_block = 0;
        last_len   = 0;
        ready_out  = 1;

        key     = 256'h603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4;
        iv_init = {96'hf0f1f2f3f4f5f6f7f8f9fafb, 32'h00000001};
        // "this!is secret" = 14 ASCII bytes
        data_in = {112'h7468697321697320736563726574, 16'h0000};
        last_len = 14;

        // Release reset
        #20 rst_n = 1;
        #10;

        // Pulse start
        @(posedge clk); start = 1;
        @(posedge clk); start = 0;

        // Wait for ready_in
        i = 0;
        while (!ready_in && i < 500) begin
            @(posedge clk); i = i + 1;
        end
        if (!ready_in) begin
            $display("[ERROR] ready_in never asserted"); $finish;
        end

        // Send partial block
        @(posedge clk);
        valid_in   = 1;
        last_block = 1;
        @(posedge clk);
        valid_in   = 0;
        last_block = 0;

        // Wait valid_out
        i = 0;
        while (!valid_out && i < 500) begin
            @(posedge clk); i = i + 1;
        end
        if (!valid_out) begin
            $display("[ERROR] valid_out never asserted"); $finish;
        end

        // Check ciphertext (MSB 14 bytes)
        if (data_out[127:16] === 112'hff551fb6332ec4420e45a1b5fd1e)
            $display("[PASS] Partial-block ciphertext correct");
        else
            $display("[FAIL] Got %h, expected ff551fb6332ec4420e45a1b5fd1e",
                     data_out[127:16]);

        // Check done
        @(posedge clk);
        if (done) $display("[PASS] done asserted");
        else      $display("[WARN] done not asserted");

        #20 $finish;
    end

endmodule
