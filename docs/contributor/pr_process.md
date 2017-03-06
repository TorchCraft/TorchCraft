 - You are USER. 
 - torchcraft/torchcraft is TC/TC.
 - TC/TC:master is the master branch on TC/TC.
 - You do a fork of TC/TC as USER/TC.
 - Nobody should ever push TC/TC:master.
 - We either:
   1) push to USER/TC:develop, merge PRs from USER/TC:develop into TC/TC:develop (that is the process with good peer review), or
   2) push to TC/TC:develop (in some cases, e.g. quick fix),
   and then at release time we merge TC/TC:develop into TC/TC:master, produce the new DLL/EXE, bundle the release.
