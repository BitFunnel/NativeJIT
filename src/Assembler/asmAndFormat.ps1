# Assembles the input file and formats the output in the format used by ml64Output in CodeGenUnitTest.cpp.
Param([string]$InputFile = "TestAsm.asm",
      [string]$OutputFile = "TestAsm.lst",
      [string]$MlPath = "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\x86_amd64\ml64.exe",
      [int]$IndentLength = 16,
      [int]$QuotedLineLength = 99)

function Expand-Tabs
{
    [CmdletBinding()]
    Param([Parameter(Position = 0,
           ValueFromPipeline = $true)]
          [string]$Text,
          [UInt16]$TabSize = 8)

    process
    {
        $expandedLine = ""
        $numExpansions = 0

        foreach ($portion in ($Text -split "`t"))
        {
            if ($numExpansions -gt 0)
            {
                $expandedLine += " " * ($TabSize - $expandedLine.Length % $TabSize)
            }

            $expandedLine += $portion
            $numExpansions++
        }

        $expandedLine
    }
}

& cmd /c "$MlPath" /nologo /c /Sn /Fl"$OutputFile" "$InputFile" | Out-Null

# Expand tabs and format as [indent]"line   \n".
Get-Content "$OutputFile" `
    | Expand-Tabs -TabSize 8 `
    | %{ "{0,-$IndentLength}`"{1,-$QuotedLineLength}\n`"" -f " ", $_ }

# Add [indent]""; to end the string.
"{0,-$IndentLength}`"`";" -f " "
