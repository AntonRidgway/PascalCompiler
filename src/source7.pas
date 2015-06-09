program semErrTest ( input, output );

var xInteger: integer;
var yInteger: integer;
var input: real;
var myReal: real;
var a: integer;
var b: array[1..1] of integer;

function fun1(a : integer;b:integer):integer;
var a: array[1..5] of integer;
	function fun2: integer;
	var a: real;
	var b: integer;
	begin
		b:= a + b;
		b:= a / b;
		b:= a mod a;
		b:= a and b;
		b:= +(a > b);
		b:= (a > b) + (b > a);
		b:= (a > b) < (b > a);
		b:= a or b;
		b:= a and b;
		b:= a[7];
		b:= a[7.890]
	end;
begin
   a := 5
end;

function fun1(apple:integer; orange:array[1..8] of integer):real;
begin
   fun1 := apple
end;

function outerFunct(intIn:integer):integer;
begin
   outerFunct := intIn * 2
end;

function noParams:integer;
begin
   noParams := 42
end;

begin
   xInteger := myReal;
   yInteger := outerFunct;
   yInteger := outerFunct(1,2);
   yInteger := fun1(1);
   yInteger := fun1(1.234,5.678);
   myReal := 12345.00000E-00;
   a[5] := InputManip(xInteger, a);
   if myReal then yInteger := 0
   while yInteger do yInteger := 0
end.