//===- EQueueOps.cpp - EQueue dialect ops ---------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include <iostream>
#include "EQueue/EQueueOps.h"
#include "EQueue/EQueueDialect.h"
#include "EQueue/EQueueTraits.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/Dialect/Traits.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/AffineExpr.h"
#include "mlir/IR/AffineMap.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Function.h"
#include "mlir/IR/FunctionImplementation.h"
#include "mlir/IR/Module.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/StandardTypes.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/EDSC/Builders.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;
using namespace xilinx::equeue;

//===----------------------------------------------------------------------===//
// CreateDMAOp 
//===-------------------------------------------------
//===----------------------------------------------------------------------===//
void CreateDMAOp::build(Builder builder, OperationState &result, StringRef name) {
	result.addAttribute("name", builder.getStringAttr(name));
	auto i32Type = IntegerType::get(32, builder.getContext());
	result.types.push_back(i32Type);
}

//===----------------------------------------------------------------------===//
// CreateMemOp 
//===----------------------------------------------------------------------===//
void CreateMemOp::build(Builder builder, OperationState &result, StringRef name, 
	ArrayRef<int64_t> shape, StringRef data, StringRef type) {
	result.addAttribute("name", builder.getStringAttr(name));
	result.addAttribute("shape", builder.getI64TensorAttr(shape));
	result.addAttribute("data", builder.getStringAttr(data));
	result.addAttribute("type", builder.getStringAttr(type));
	auto i32Type = IntegerType::get(32, builder.getContext());
	result.types.push_back(i32Type);
}

static ParseResult parseCreateMemOp(OpAsmParser &parser,
                                     OperationState &result) {
	Attribute extentsRaw;
  StringRef name, data, type;
	NamedAttrList dummy;
	if (parser.parseKeyword(&name) || parser.parseComma() || 
	    parser.parseAttribute(extentsRaw, "shape", dummy) || 
			parser.parseComma() || parser.parseKeyword(&data) || 
			parser.parseComma() || parser.parseKeyword(&type))
		return failure();
	auto extentsArray = extentsRaw.dyn_cast<ArrayAttr>();
	if (!extentsArray)
		return failure();
	SmallVector<int64_t, 6> ints;
	for (Attribute extent : extentsArray) {
		IntegerAttr attr = extent.dyn_cast<IntegerAttr>();
		if (!attr)
		return failure();
		ints.push_back(attr.getInt());
	}
	Builder &builder = parser.getBuilder();
	result.addAttribute("name",  parser.getBuilder().getStringAttr(name));
	result.addAttribute("shape", builder.getI64TensorAttr(ints));
	result.addAttribute("data", parser.getBuilder().getStringAttr(data));
	result.addAttribute("type", parser.getBuilder().getStringAttr(type));
	auto i32Type = IntegerType::get(32, builder.getContext());
	result.types.push_back(i32Type);
	return success();
}


//===----------------------------------------------------------------------===//
// CreateProcOp 
//===----------------------------------------------------------------------===//
void CreateProcOp::build(Builder builder, OperationState &result, StringRef name, 
  StringRef type) {
	result.addAttribute("name", builder.getStringAttr(name));
	result.addAttribute("type", builder.getStringAttr(type));
	auto i32Type = IntegerType::get(32, builder.getContext());
	result.types.push_back(i32Type);
}
static ParseResult parseCreateProcOp(OpAsmParser &parser,
                                     OperationState &result) {
	StringRef name, type;
	if (parser.parseKeyword(&name) || parser.parseComma() || parser.parseKeyword(&type) )
		return failure();

	Builder &builder = parser.getBuilder();
  result.addAttribute("name", parser.getBuilder().getStringAttr(name));
  result.addAttribute("type", parser.getBuilder().getStringAttr(type));
  auto i32Type = IntegerType::get(32, builder.getContext());
  result.types.push_back(i32Type);
	return success();
}
//===----------------------------------------------------------------------===//
// CreateCompOp 
//===----------------------------------------------------------------------===//
void CreateCompOp::build(Builder builder, OperationState &result, ValueRange comps, StringRef name) {
  result.addOperands(comps);
	result.addAttribute("name", builder.getStringAttr(name));
	auto i32Type = IntegerType::get(32, builder.getContext());
	result.types.push_back(i32Type);
}

//===----------------------------------------------------------------------===//
// GetCompOp 
//===----------------------------------------------------------------------===//
void GetCompOp::build(Builder builder, OperationState &result, Value comp, StringRef name, 
  StringRef type) {
  result.addOperands(comp);
	result.addAttribute("name", builder.getStringAttr(name));
	auto i32Type = IntegerType::get(32, builder.getContext());
	result.types.push_back(i32Type);
}

//===----------------------------------------------------------------------===//
// MemAllocOp 
//===----------------------------------------------------------------------===//
void MemAllocOp::build(Builder builder, OperationState &result, Value mem, 
	ArrayRef<int64_t> shape, StringRef data, Type tensorDataType) {
  result.addOperands(mem);
	result.addAttribute("shape", builder.getI64TensorAttr(shape));
	result.addAttribute("data", builder.getStringAttr(data));
  auto tensorType =  RankedTensorType::get(
        shape, tensorDataType);
	auto i32Type = IntegerType::get(32, builder.getContext());
	auto containerType = EQueueContainerType::get(tensorType, i32Type);
	result.types.push_back(containerType);
}

static ParseResult parseMemAllocOp(OpAsmParser &parser,
                                     OperationState &result) {
	Builder &builder = parser.getBuilder();
  OpAsmParser::OperandType mem;
  Attribute extentsRaw;
  StringRef data;
	NamedAttrList dummy;
	auto i32Type = IntegerType::get(32, builder.getContext());
  Type resType;
	if ( parser.parseOperand(mem) || parser.parseComma() ||
		parser.resolveOperand(mem, i32Type, result.operands) || 
		parser.parseAttribute(extentsRaw, "shape", dummy) ||
		parser.parseComma() ||
		parser.parseKeyword(&data) ||
		parser.parseColon() ||
		parser.parseType(resType) ) 
    return failure();
	result.addAttribute("data", parser.getBuilder().getStringAttr(data));
	result.types.push_back(resType);
	auto extentsArray = extentsRaw.dyn_cast<ArrayAttr>();
	if (!extentsArray)
		return failure();
	SmallVector<int64_t, 6> ints;
	for (Attribute extent : extentsArray) {
		IntegerAttr attr = extent.dyn_cast<IntegerAttr>();
		if (!attr)
		return failure();
		ints.push_back(attr.getInt());
	}
	result.addAttribute("shape", builder.getI64TensorAttr(ints));
	return success();
}

//===----------------------------------------------------------------------===//
// MemDeallocOp 
//===----------------------------------------------------------------------===//
void MemDeallocOp::build(Builder builder, OperationState &result, ValueRange buffer) {
  result.addOperands(buffer);
}

static ParseResult parseMemDeallocOp(OpAsmParser &parser,
                                     OperationState &result) {
  Builder &builder = parser.getBuilder();
	//OpAsmParser::OperandType signal;
	//auto signalType = EQueueSignalType::get(builder.getContext());
 	SmallVector<OpAsmParser::OperandType, 8> buffers;
	SmallVector<Type, 8> types;
  if (  //parser.parseOperand(signal) || parser.parseComma() || 
		//parser.resolveOperand(signal, signalType, result.operands) ||
		parser.parseOperandList(buffers) ||  
		parser.parseColonTypeList(types) ||
		parser.resolveOperands(buffers, types, parser.getCurrentLocation(), 
		result.operands) )
    return failure();
	//result.types.push_back(signalType);
	return success();
}

//===----------------------------------------------------------------------===//
// MemWriteOp 
//===----------------------------------------------------------------------===//
void MemWriteOp::build(Builder builder, OperationState &result, Value value, ValueRange buffer) {
  result.addOperands(value);
  result.addOperands(buffer);
}
//===----------------------------------------------------------------------===//
// MemWriteOp 
//===----------------------------------------------------------------------===//
//XXX(Zhijing): tensorType does not store elementtype, so there is no "get" function to get the elementype get
// the only thing we can do is to explicit give a type
void MemReadOp::build(Builder builder, OperationState &result, Value container, ValueRange index, Type type) {
  result.addOperands(container);
  result.addOperands(index);
  if(index.size() >= 1){
	  result.types.push_back(type);//scalar
	}else{
	  result.types.push_back(container.getType().cast<EQueueContainerType>().getValueType());//tensor
	}
	
}

//===----------------------------------------------------------------------===//
// LaunchOp 
//===----------------------------------------------------------------------===//
//XXX(Zhijing): not sure about bodyBuilder yet
// also, why opbuilder, not builder, what's the difference?
void LaunchOp::build(OpBuilder builder, OperationState &result, Value start, Value device,
  ValueRange operands, function_ref<void(OpBuilder &, Location, ValueRange)> bodyBuilder) {
  result.addOperands(start);
  result.addOperands(device);
  result.addOperands(operands);
  Region *bodyRegion = result.addRegion();
  Block &bodyBlock = bodyRegion->front();
  for(auto operand: operands){
    bodyBlock.addArgument(operand.getType());
  }

	auto signalType = EQueueSignalType::get(builder.getContext());
	result.types.push_back(signalType); 
  
  OpBuilder::InsertionGuard guard(builder);
  builder.setInsertionPointToStart(&bodyBlock);
  bodyBuilder(builder, result.location, bodyBlock.getArguments());
}
static ParseResult parseLaunchOp(OpAsmParser &parser,
                                     OperationState &result) {
	Builder &builder = parser.getBuilder();
	SmallVector<OpAsmParser::OperandType, 8> regionArgs;
	SmallVector<OpAsmParser::OperandType, 10> operands;
	OpAsmParser::OperandType regionArg, device, signal;
	SmallVector<Type, 8> types;
	SmallVector<Type, 8> resTypes;
 	if ( parser.parseLParen() ) return failure();
	
	while (succeeded( parser.parseOptionalRegionArgument(regionArg)) &&
		!regionArg.name.empty()) {
		regionArgs.push_back(regionArg);
		if (failed(parser.parseOptionalComma())) {
			if (parser.parseEqual() || 
				parser.parseOperandList(operands) ||
				parser.parseColonTypeList(types)) 
				return failure();
			break;
		}
	}
	
	if (parser.parseRParen() ||
		parser.parseKeyword("in") ||
		parser.parseLParen() ||
		parser.parseOperand(signal) || 
		parser.parseComma() || 
		parser.parseOperand(device) || 
		parser.parseRParen() ||
		parser.parseOptionalColonTypeList(resTypes))
		return failure();

	Region *body = result.addRegion();
	if (operands.size() != regionArgs.size() || parser.parseRegion(*body, regionArgs, 
			types) )
		return failure();

	operands.insert(operands.begin(), device);
	operands.insert(operands.begin(), signal);

	auto i32Type = IntegerType::get(32, builder.getContext());
	auto signalType = EQueueSignalType::get(builder.getContext());
	types.insert(types.begin(), i32Type);
	types.insert(types.begin(), signalType);
	if ( parser.resolveOperands(operands, types, parser.getCurrentLocation(), 
		result.operands)) 
		return failure();
	result.types.push_back(signalType); 
	result.types.append(resTypes.begin(), resTypes.end()); 
	return success();
}


//===----------------------------------------------------------------------===//
// MemCopyOp 
//===----------------------------------------------------------------------===//
void MemCopyOp::build(Builder builder, OperationState &result, Value start, Value src_buffer, Value dest_buffer, Value dma, ValueRange offset) {
  result.addOperands(start);
  result.addOperands(src_buffer);
  result.addOperands(dest_buffer);
  result.addOperands(dma);
  result.addOperands(offset);
	auto signalType = EQueueSignalType::get(builder.getContext());
  result.types.push_back(signalType);
}

//===----------------------------------------------------------------------===//
// ControlStartOp 
//===----------------------------------------------------------------------===//
void ControlStartOp::build(Builder builder, OperationState &result) {
	auto signalType = EQueueSignalType::get(builder.getContext());
  result.types.push_back(signalType);
}


//===----------------------------------------------------------------------===//
// ControlAndOp 
//===----------------------------------------------------------------------===//
void ControlAndOp::build(Builder builder, OperationState &result, ValueRange signals) {
  result.addOperands(signals);
	auto signalType = EQueueSignalType::get(builder.getContext());
  result.types.push_back(signalType);
}


//===----------------------------------------------------------------------===//
// ControlOrOp 
//===----------------------------------------------------------------------===//
void ControlOrOp::build(Builder builder, OperationState &result, ValueRange signals) {
  result.addOperands(signals);
	auto signalType = EQueueSignalType::get(builder.getContext());
  result.types.push_back(signalType);
}


//===----------------------------------------------------------------------===//
// AwaitOp 
//===----------------------------------------------------------------------===//
void AwaitOp::build(Builder builder, OperationState &result, ValueRange signals) {
  result.addOperands(signals);
}
namespace xilinx {
namespace equeue {
#define GET_OP_CLASSES
#include "EQueue/EQueueOps.cpp.inc"
} // namespace equeue
} // namespace xilinx
