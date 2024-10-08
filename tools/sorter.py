import json
import sys

data = json.loads(sys.stdin.read())

data['allocs'] = list(sorted(data['allocs'], key=lambda x: -x['size']))

print(json.dumps(data))
