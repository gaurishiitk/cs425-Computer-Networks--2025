# DNS Resolution - Iterative and Recursive Lookup

## Overview

This project implements a DNS resolution system that supports both iterative and recursive lookups. It is part of the CS425: Computer Networks course assignment, aimed at providing hands-on experience with DNS querying, network programming, and protocol understanding.

## Assignment Features

### How to Run the DNS Resolver

1. **Install dependencies**
   ```
   pip install dnspython
   ```

2. **Run the resolver**
   - For iterative lookup:
     ```
     python3 dnsresolver.py iterative example.com
     ```
   - For recursive lookup:
     ```
     python3 dnsresolver.py recursive example.com
     ```

### Implemented Features:
- Iterative DNS resolution starting from root servers
- Recursive DNS resolution using system's default resolver
- Support for A record types
- Error handling for timeouts, incorrect domain names, and unreachable servers
- Debug output for each step of the resolution process

### Not Implemented Features:
- Support for other DNS record types (MX, TXT, etc.)
- Caching of DNS responses

### DNS Query Approach
- Used `dnspython` library for constructing and parsing DNS messages
- Implemented separate functions for iterative and recursive lookups

## Implementation Details

### High-Level Idea of Important Functions:

1. **`iterative_lookup(domain)`**:
   - Starts with root DNS servers
   - Iteratively queries nameservers until reaching authoritative server
   - Returns resolved IP address

2. **`recursive_lookup(domain)`**:
   - Uses system's default resolver for recursive lookup
   - Returns resolved IP address

3. **`query_dns(server, domain, record_type)`**:
   - Sends DNS query to specified server
   
4. **`extract_nameserver(response)`**:
   - Extracts nameserver information from DNS response

5. **`resolve_nameserver(ns_hostname)`**:
   - Resolves IP address of a nameserver hostname

### Code Flow
1. **Parse command-line arguments**
2. **Based on lookup type:**
   - **Iterative:** Start from root, query through TLD and authoritative servers
   - **Recursive:** Use system resolver
3. **Print results and execution time**

### Changes made

I have made the following changes to the code and here's an explanation of each:

1. In the `send_dns_query` function:
   - Implemented UDP query using `dns.query.udp_with_fallback()`.
   - This method handles potential UDP message truncation by falling back to TCP if necessary.
   - The response is returned as the first element of the tuple.

2. In the `extract_next_nameservers` function:
   - Added a loop to resolve NS hostnames to IP addresses.
   - It checks the additional section of the DNS response for A records.
   - Extracts the hostname and IP address, appends to `ns_ips` list, and prints the resolution.

3. In the `iterative_dns_lookup` function:
   - Implemented stage progression from ROOT to TLD to AUTH.
   - This allows the function to track which level of DNS hierarchy it's currently querying.

4. In the `recursive_dns_lookup` function:
   - Used `dns.resolver.resolve()` to perform recursive DNS lookups.
   - First resolves NS records, then A records for the domain.
   - Prints each successful resolution.

## Steps to Execute the Code

1. Ensure you have Python 3.x installed.
2. Install the required library:
   ```
   pip install dnspython
   ```
3. Save the provided code as `dnsresolver.py`.
4. Run the script from the command line:
   - For iterative lookup:
     ```
     python3 dnsresolver.py iterative example.com
     ```
   - For recursive lookup:
     ```
     python3 dnsresolver.py recursive example.com
     ```
5. The script will output the resolution process and results, along with the time taken for execution.

## Contribution of Team Members

| Member |
|--------|
| Madhur Bansal (210572) |
| Lakshika (210554) |
| Gaurish Bansal (210390) |

## References
- DNS protocol specifications (RFC 1034, 1035)
- `dnspython` library documentation
- Course materials on DNS resolution

## Declaration
We hereby declare that we did not indulge in any plagiarism while completing this assignment.
