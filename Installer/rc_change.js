var src = WScript.Arguments(0);
var dst = WScript.Arguments(1);
var re = new RegExp(src, "g");

while (!WScript.StdIn.AtEndOfStream){
	var line = WScript.StdIn.ReadLine();
	WScript.StdOut.Write(line.replace(re, dst) + "\n");
}
