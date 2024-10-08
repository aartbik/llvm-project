; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -passes=instcombine -S | FileCheck %s

define i32 @foo(i32 %x, i32 %y) {
; CHECK-LABEL: @foo(
; CHECK-NEXT:    [[RES:%.*]] = mul i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    ret i32 [[RES]]
;
  %add = add nsw i32 %y, %x
  %mul = mul nsw i32 %add, %y
  %square = mul nsw i32 %y, %y
  %res = sub i32 %mul, %square
  ret i32 %res
}

define i1 @bar(i64 %x, i64 %y) {
; CHECK-LABEL: @bar(
; CHECK-NEXT:    [[Y1:%.*]] = xor i64 [[X:%.*]], -1
; CHECK-NEXT:    [[B:%.*]] = and i64 [[Y:%.*]], [[Y1]]
; CHECK-NEXT:    [[R:%.*]] = icmp eq i64 [[B]], 0
; CHECK-NEXT:    ret i1 [[R]]
;
  %a = and i64 %y, %x
  %not = xor i64 %a, -1
  %b = and i64 %y, %not
  %r = icmp eq i64 %b, 0
  ret i1 %r
}
