; ModuleID = 'prog'
source_filename = "prog"

@0 = private unnamed_addr constant [3 x i8] c"%f\00", align 1

define i32 @main() {
program:
  %out = alloca i32, align 4
  %tmp2 = alloca double, align 8
  %value = alloca double, align 8
  store double 0.000000e+00, double* %value, align 8
  store double 0.000000e+00, double* %tmp2, align 8
  store i32 0, i32* %out, align 4
  store double 1.500000e+01, double* %value, align 8
  %value1 = load double, double* %value, align 8
  %addtmp = fadd double %value1, 5.000000e+01
  store double %addtmp, double* %tmp2, align 8
  %tmp22 = load double, double* %tmp2, align 8
  %calltmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), double %tmp22)
  store i32 %calltmp, i32* %out, align 4
  ret i32 0
}

declare i32 @printf(i8*, ...)
