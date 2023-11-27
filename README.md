# GaNTracer - Git repository for 2023 capstone

## Python venv
Python provides a nice way of using modules without needing to install them
system wide, called venv's. I'm not well versed in the subject, but this should
be enough to get going for now:

create a venv in directory `.venv`:
`python -m venv .venv`

Activate the venv:
POSIX-ish: `source .venv/bin/activate`
Windows: `.venv/bin/Activate.bat`
Your prompt should change to be prefixed with (.venv) if successful

Install any required modules to venv (if the file exists):
`pip install -r requirements.txt --upgrade`

Once finished in the venv, deactivate by just running `deactivate`
