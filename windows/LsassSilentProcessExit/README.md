# LsassSilentProcessExit
New method of causing WerFault.exe to dump lsass.exe process memory to disk for credentials extraction via silent process exit mechanism without crasing lsass.exe.

    Usage:
    LsassSilentProcessExit.exe <PID of LSASS.exe> <DumpMode>
  
    Where DumpMode can be:

      0 - Call RtlSilentProcessExit on LSASS process handle
      1 - Call CreateRemoteThread on RtlSilentProcessExit on LSASS
