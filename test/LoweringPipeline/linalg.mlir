#map0 = affine_map<(d0, d1, d2, d3) -> (d0 * 600 + d1 * 60 + d2 * 6 + d3)>
#map1 = affine_map<(d0, d1, d2, d3, d4, d5, d6) -> (d0, d2, d3, d6)>
#map2 = affine_map<(d0, d1, d2, d3, d4, d5, d6) -> (d4, d5, d6, d1)>
#map3 = affine_map<(d0, d1, d2, d3, d4, d5, d6) -> (d0, d2, d3, d1)>


module {
  func @graph() {
    %0 = "equeue.create_proc"() {type = "AIEngine"} : () -> i32
    %1 = "equeue.create_mem"() {banks = 1 : i64, data_bit = 32 : i64, shape = dense<3> : tensor<1xi64>, type = "RegisterFile"} : () -> i32
    %2 = "equeue.create_dma"() : () -> i32
    %3 = "equeue.create_comp_field"(%0, %1, %2) {names = "proc mem dma "} : (i32, i32, i32) -> i32
    %4 = splat %3 : vector<12x14xi32>
    %5 = "equeue.create_mem"() {banks = 16 : i64, data_bit = 32 : i64, shape = dense<4096> : tensor<1xi64>, type = "SRAM"} : () -> i32
    %6 = "equeue.create_proc"() {type = "MicroPlate"} : () -> i32
    %7 = "equeue.create_comp_field"(%4, %6, %5, %2) {names = "pe_array proc mem "} : (vector<12x14xi32>, i32, i32, i32) -> i32
    %8 = "equeue.control_start"() : () -> !equeue.signal
    %done = "equeue.launch"(%8, %6, %7) ( {
    ^bb0(%arg0: i32):  // no predecessors
      %9 = "equeue.get_comp_field"(%arg0) {name = "mem"} : (i32) -> i32
      %10 = "equeue.alloc"(%9) {data_bit = 32 : i64, shape = dense<[1, 10, 10, 6]> : tensor<4xi64>} : (i32) -> memref<1x10x10x6xf32>
      "equeue.add_comp_field"(%arg0, %10) {names = "ibuffer "} : (i32, memref<1x10x10x6xf32>) -> ()
      %11 = "equeue.alloc"(%9) {data_bit = 32 : i64, shape = dense<[3, 3, 6, 10]> : tensor<4xi64>} : (i32) -> memref<3x3x6x10xf32>
      "equeue.add_comp_field"(%arg0, %11) {names = "wbuffer "} : (i32, memref<3x3x6x10xf32>) -> ()
      %12 = "equeue.alloc"(%9) {data_bit = 32 : i64, shape = dense<[1, 8, 8, 10]> : tensor<4xi64>} : (i32) -> memref<1x8x8x10xf32>
      "equeue.add_comp_field"(%arg0, %12) {names = "obuffer "} : (i32, memref<1x8x8x10xf32>) -> ()
      %13 = subview %10[0, 0, 0, 0] [1, 8, 8, 6] [1, 1, 1, 1]  : memref<1x10x10x6xf32> to memref<1x8x8x6xf32, #map0>
      linalg.generic {args_in = 2 : i64, args_out = 1 : i64, indexing_maps = [#map1, #map2, #map3], iterator_types = ["parallel", "parallel", "parallel", "parallel", "parallel", "parallel", "parallel"]} %13, %11, %12 {
      ^bb0(%arg1: f32, %arg2: f32, %arg3: f32):  // no predecessors
        %14 = "equeue.unkOp"(%arg3, %arg1, %arg2) {op_name = "mac"} : (f32, f32, f32) -> f32
        linalg.yield %14 : f32
      }: memref<1x8x8x6xf32, #map0>, memref<3x3x6x10xf32>, memref<1x8x8x10xf32>
      "equeue.return"() : () -> ()
    }) : (!equeue.signal, i32, i32) -> !equeue.signal
    return
  }
}
