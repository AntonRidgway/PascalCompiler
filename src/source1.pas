program goodTest ( input, output );

var xInteger: integer;
var yInteger: integer;
var myReal: real;
var a: array[123456789..7.2] of integer;

function recursiveF(a : integer;b:integer):integer;
begin
   if b = 0 then a := a * b;
   if a <> b then recursiveF := a; if a + b > 5
   then recursiveF := recursiveF * 2
   else recursiveF := recursiveF(b, a mod b)
end;

function InputManip(apple:integer; orange:array[1..8] of integer):real;
begin
   apple := apple + orange[1];
   apple := apple - orange[2];
   apple := apple or orange[3];
   apple := apple * orange[4];
   apple := apple / orange[5];
   apple := (apple div orange[6]);
   apple := apple mod orange[7];
   apple := apple and orange[8];
   InputManip := apple
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
   xInteger := recursiveF(x,y);
   yInteger := outerFunct(recursiveF(x,y));
   myReal := 12345.00000E-00;
   myReal := InputManip(xInteger, a);
   while yInteger < xInteger do yInteger := 0
end.