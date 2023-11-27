# GaNTracer - Git repository for 2023 capstone

## Extremely basic git overview

When you want to implement a new feature/program/thing:

1. Make sure you are on the main branch before branching off:  
`$ git checkout main`  

2. Get latest version from upstream:  
`$ git pull`  

3. Create a new branch called `<feature>`:  
`$ git branch <feature>`

4. Switch to your new branch:  
`$ git checkout <feature>`

5. Let remote know about it's existance:  
`$ git push --set-upstream origin test`

6. Do your development work and commit along the way:  
`$ git add <file(s)>`  
`$ git commit`  
`$ git push`  

7. When the feature is finished, merge it into main branch:  
`$ git checkout main`  
`$ git merge <feature>`  
`$ git push`  

## CCS workspace
Not entirely sure if this will work, but theoretically we can just use the one
ccs workspace for everyone

### Setup ccs workspace
1. Clone this repo: `$ git clone git@github.com:henry-msu/GaNTracer`
2. Open CCS
3. If it prompts you to select a workspace, navigate to wherever this repo is
   cloned and choose `GaNTracer/ccs workspace`
4. If it doesn't prompt, click `File` > `Switch Workspace` > `Other` and follow
   step 3

## Python venvs
Python provides a nice way of using modules without needing to install them
system wide, called venv's. I'm not well versed in the subject, but this should
be enough to get going for now:

### Create a venv in directory `.venv`:

`$ python -m venv .venv`

### Activate the venv:

* POSIX-ish: `$ source .venv/bin/activate`
* Windows: `C:\> .venv/bin/Activate.bat`

Your prompt should change to be prefixed with (.venv) if successful

### Install any required modules to venv (if the file exists):

`$ pip install -r requirements.txt --upgrade`

### Deactivate venv
Once finished in the venv, deactivate it:
`$ deactivate`
