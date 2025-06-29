// File: plaintext_splitter.v

module byte_fifo #(
    parameter ADDR_WIDTH = 8  // DEPTH = 2^ADDR_WIDTH
)(
    input               clk,
    input               rst,
    // Write side
    input       [7:0]   data_in,
    input               write_en,
    output              full,
    // Read side
    output reg [7:0]    data_out,
    input               read_en,
    output              empty,
    // Occupancy 
    output reg [ADDR_WIDTH:0] count
);
    localparam DEPTH = (1 << ADDR_WIDTH);

    reg [7:0] mem [0:DEPTH-1];
    reg [ADDR_WIDTH-1:0] wr_ptr;
    reg [ADDR_WIDTH-1:0] rd_ptr;

    assign full  = (count == DEPTH);
    assign empty = (count == 0);

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            wr_ptr   <= 0;
            rd_ptr   <= 0;
            count    <= 0;
            data_out <= 8'd0;
        end else begin
            // Write
            if (write_en && !full) begin
                mem[wr_ptr] <= data_in;
                wr_ptr      <= wr_ptr + 1;
                count       <= count + 1;
            end
            // Read
            if (read_en && !empty) begin
                data_out <= mem[rd_ptr];
                rd_ptr   <= rd_ptr + 1;
                count    <= count - 1;
            end
          
        end
    end
endmodule


module plaintext_splitter (
    input         clk,
    input         rst,
    input  [7:0]  data_byte,
    input         valid_byte,
    input         data_done,
    output        ready_byte,

    output reg [127:0] data_block,
    output reg         valid_block,
    input              ready_block,
    output reg         last_block,
    output reg [6:0]   partial_len
);
    reg [127:0] buffer;
    reg [6:0]   bit_count;
    reg         eof_pending;

    assign ready_byte = (bit_count < 128) && !valid_block;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            buffer       <= 0;
            bit_count    <= 0;
            valid_block  <= 0;
            last_block   <= 0;
            partial_len  <= 0;
            eof_pending  <= 0;
        end else begin
            valid_block <= 0; 
            if (valid_byte && ready_byte) begin
                buffer    <= {buffer[119:0], data_byte};
                bit_count <= bit_count + 8;
            end

            if ((bit_count == 120) && valid_byte && data_done) begin
                eof_pending <= 1;
            end

            if (!valid_block && bit_count == 128) begin
                data_block  <= buffer;
                valid_block <= 1;
                last_block  <= eof_pending;
                partial_len <= 0;
                eof_pending <= 0;
            end
            else if (!valid_block && data_done && bit_count > 0) begin
                data_block  <= buffer;
                valid_block <= 1;
                last_block  <= 1;
                partial_len <= bit_count;
            end

            if (valid_block && ready_block) begin
                bit_count   <= 0;
                last_block  <= 0;
                partial_len <= 0;
                eof_pending <= 0;
            end
        end
    end
endmodule


module splitter_with_fifo #(
    parameter FIFO_ADDR_WIDTH = 8
)(
    input         clk,
    input         rst,
    // Up-stream byte stream
    input  [7:0]  data_byte_in,
    input         valid_byte_in,
    input         data_done_in,
    output        ready_byte_up,

    // Down-stream block stream
    output [127:0] data_block,
    output         valid_block,
    input          ready_block,
    output         last_block,
    output [6:0]   partial_len
);
    // ---------- FIFO signals ----------
    wire fifo_full, fifo_empty;
    wire [7:0] fifo_data_out;
    wire [FIFO_ADDR_WIDTH:0] fifo_count;

    wire fifo_write_en = valid_byte_in && !fifo_full;
    assign ready_byte_up = !fifo_full;
    reg fifo_read_en_d;

    byte_fifo #(
        .ADDR_WIDTH(FIFO_ADDR_WIDTH)
    ) u_fifo (
        .clk      (clk),
        .rst      (rst),
        .data_in  (data_byte_in),
        .write_en (fifo_write_en),
        .full     (fifo_full),
        .data_out (fifo_data_out),
        .read_en  (fifo_read_en_d),
        .empty    (fifo_empty),
        .count    (fifo_count)
    );

    reg upstream_done;
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            upstream_done <= 1'b0;
        end else begin
            if (valid_byte_in && !fifo_full && data_done_in) begin
                upstream_done <= 1'b1;
            end
            if (valid_block && ready_block && last_block) begin
                upstream_done <= 1'b0;
            end
        end
    end

    wire splitter_ready;
    reg  valid_byte_to_splitter;
    reg  data_done_to_splitter;
    reg [7:0] data_byte_to_splitter;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            fifo_read_en_d           <= 1'b0;
            valid_byte_to_splitter   <= 1'b0;
            data_done_to_splitter    <= 1'b0;
            data_byte_to_splitter    <= 8'd0;
        end else begin
            fifo_read_en_d           <= 1'b0;
            valid_byte_to_splitter   <= 1'b0;
            data_done_to_splitter    <= 1'b0;
            data_byte_to_splitter    <= 8'd0;
            if (!fifo_empty && splitter_ready) begin
                fifo_read_en_d         <= 1'b1;
                valid_byte_to_splitter <= 1'b1;
                data_byte_to_splitter  <= fifo_data_out;
                if (upstream_done && fifo_count == 1) begin
                    data_done_to_splitter <= 1'b1;
                end
            end
        end
    end

    // Instatiate plaintext_splitter
    plaintext_splitter u_splitter (
        .clk        (clk),
        .rst        (rst),
        .data_byte  (data_byte_to_splitter),
        .valid_byte (valid_byte_to_splitter),
        .data_done  (data_done_to_splitter),
        .ready_byte (splitter_ready),
        .data_block (data_block),
        .valid_block(valid_block),
        .ready_block(ready_block),
        .last_block (last_block),
        .partial_len(partial_len)
    );

endmodule

