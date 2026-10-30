# compare.py <base.log> <after.log> <base-rev>: key = (file, source line text, message); multiset diff
import re,sys,subprocess,collections,os
R='/home/lhen/Projects/ryoku-work/kwinish-port-0-57'
def src(path,rev):
    if path.startswith('/'): return open(path,errors='replace').read().splitlines()
    if rev: return subprocess.run(['git','-C',R,'show',f'{rev}:plugin/{path}'],capture_output=True,text=True).stdout.splitlines()
    return open(os.path.join(R,'plugin',path)).read().splitlines()
def keys(log,rev):
    c=collections.Counter(); cache={}
    for l in open(log,errors='replace'):
        m=re.match(r'^(\S+?):(\d+):\d+: error: (.*)$',l.rstrip())
        if not m: continue
        f,n,msg=m.group(1).removeprefix('./'),int(m.group(2)),m.group(3)
        if f not in cache: cache[f]=src(f,rev)
        line=cache[f][n-1].strip() if n<=len(cache[f]) else '?'
        msg=msg.replace('‘',"'").replace('’',"'")
        c[(f,line,msg)]+=1
    return c
b=keys(sys.argv[1],sys.argv[3]); a=keys(sys.argv[2],None)
print(f'base={sum(b.values())} after={sum(a.values())}')
gone=b-a; new=a-b
print(f'removed={sum(gone.values())}'); [print('  -',k) for k in gone]
print(f'new={sum(new.values())}'); [print('  +',k) for k in new]
