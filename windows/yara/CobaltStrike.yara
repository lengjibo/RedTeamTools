import "pe"
rule CobaltStrike4_1:beacon
{
    meta:
        description = "This rule is used to detect CobaltStrike4.1 Trojan"
        author = "lengyi"
    strings:
         $name = "%c%c%c%c%c%c%c%c%cMSSE-%d-server"
    condition:
         uint16(0) == 0x5A4D and pe.entry_point ==0x8b0 and filesize > 10KB and filesize < 54KB and $name
}