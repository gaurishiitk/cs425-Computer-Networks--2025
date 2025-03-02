
# DNS Resolution - Iterative and Recursive Lookup

## Overview

This project implements a DNS resolution system that supports both iterative and recursive lookups. It is part of the CS425: Computer Networks course assignment, aimed at providing hands-on experience with DNS querying, network programming, and protocol understanding.

## Features

- Iterative DNS Resolution
- Recursive DNS Resolution
- Error handling for timeouts, incorrect domain names, and unreachable servers

## Requirements

- Python 3.x
- dnspython library

## Installation

1. Clone the repository:
   ```
   git clone https://github.com/privacy-iitk/cs425-2025.git
   ```

2. Navigate to the assignment directory:
   ```
   cd cs425-2025/Homeworks/A2
   ```

3. Install the required library:
   ```
   pip install dnspython
   ```

## Usage

The script accepts command-line arguments for both iterative and recursive DNS lookups:

### Iterative Lookup
```
python3 dnsresolver.py iterative example.com
```

### Recursive Lookup
```
python3 dnsresolver.py recursive example.com
```

## Implementation Details

### Iterative DNS Resolution
- Starts resolution from root DNS servers
- Queries through TLD and authoritative servers
- Extracts and resolves nameservers at each stage
- Prints the resolved IP address if found

### Recursive DNS Resolution
- Uses the system's default DNS resolver
- Fetches and displays the IP address of the given domain
- Handles errors gracefully

## Example Output

### Iterative Lookup
```
[Iterative DNS Lookup] Resolving google.com
[DEBUG] Querying ROOT server (198.41.0.4) - SUCCESS
Extracted NS hostname: l.gtld-servers.net.
...
[DEBUG] Querying TLD server (192.41.162.30) - SUCCESS
Extracted NS hostname: ns2.google.com.
...
[DEBUG] Querying AUTH server (216.239.34.10) - SUCCESS
[SUCCESS] google.com -> 142.250.194.78
Time taken: 0.597 seconds
```

### Recursive Lookup
```
[Recursive DNS Lookup] Resolving google.com
[SUCCESS] google.com -> ns4.google.com.
[SUCCESS] google.com -> ns3.google.com.
[SUCCESS] google.com -> ns2.google.com.
[SUCCESS] google.com -> ns1.google.com.
[SUCCESS] google.com -> 172.217.167.206
Time taken: 0.014 seconds
```

## File Structure

- `dnsresolver.py`: Main Python script containing the DNS resolution implementation
- `README.md`: This file, containing project documentation

## Troubleshooting

- If you encounter "Name or service not known" errors, check your internet connection and DNS settings
- For "No answer" responses, the domain might not have an A record or could be invalid

## Contributors

This project is completed as part of the CS425: Computer Networks course at IIT Kanpur.

This markdown content provides a comprehensive README for your DNS Resolution project, including an overview, features, installation instructions, usage examples, implementation details, example outputs, file structure, troubleshooting tips, and submission guidelines. You can copy and paste this content directly into your README.md file.

Citations:
[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/51750180/96ce1578-4a7a-4bd7-9706-af22ee994286/A2_528c41fe-3811-45cd-b4ea-229a72ae79ba.pdf
[2] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/51750180/662e8595-84ab-46ba-a103-5573d84fc442/dnsresolver_mine.py
