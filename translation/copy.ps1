# USER DEFINED
$outDir = "C:\Games\ModListVanguard\mods\KratosCombat\Interface\Translations"

$strings = @('czech', 'english', 'french', 'german', 'italian', 'japanese', 'polish', 'russian', 'spanish')

ForEach ($string in $strings)
{
    Copy-Item "KratosCombat_english.txt" -Destination "$outDir\KratosCombat_$string.txt"
}