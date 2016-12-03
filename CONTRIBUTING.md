# Contributing to TorchCraft
We want to make contributing to this project as easy and transparent as
possible. The `master` branch on this version is considered the source of
truth. All new development of TorchCraft should end up as commits or pull
requests here.

## Our Development Process

We try to sync as frequently as possible between this version of TorchCraft and
the Facebook internal one. This version is considered the source of truth.

## Pull Requests
We actively welcome your pull requests.

1. Fork the repo and create your branch from `master`.
2. Reinstall `luarocks make torchcraft-*.rockspec`
3. Ensure that at least `examples/simple_dll.lua`, `examples/simple_exe.lua`,
   and `examples/baseline_heuristics.lua` run properly in `micro_mode` on
`maps/micro/m5v5_c_far.scm` (run it in "Use Map Settings", and that
`examples/simple_dll.lua` runs properly without `micro_mode` on `(2)Astral
Balance.scm` (it comes with the game) playing as Terran in Melee mode.
4. (If you've added code that can be tested, add tests.)
5. (If you've changed APIs, update the documentation.)
6. If you haven't already, complete the Contributor License Agreement ("CLA").

## Contributor License Agreement ("CLA")
In order to accept your pull request, we need you to submit a CLA. You only need
to do this once to work on any of Facebook's open source projects.

Complete your CLA here: <https://code.facebook.com/cla>

## Issues
We use GitHub issues to track public bugs. Please ensure your description is
clear and has sufficient instructions to be able to reproduce the issue.

<!--Facebook has a [bounty program](https://www.facebook.com/whitehat/) for the safe
disclosure of security bugs. In those cases, please go through the process
outlined on that page and do not file a public issue.-->

## Coding Style  
* for C++: 2 spaces for indentation rather than tabs
* for Lua: 4 spaces for indentation rather than tabs
* line endings: LF on all files in the repo (be careful not to insert CR LF
  from Windows, configure your editor and/or git client)

## License
By contributing to TorchCraft, you agree that your contributions will be licensed
under the LICENSE file in the root directory of this source tree.
