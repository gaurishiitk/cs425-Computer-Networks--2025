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
- Support for both A and CNAME record types
- Error handling for timeouts, incorrect domain names, and unreachable servers
- Debug output for each step of the resolution process

### Not Implemented Features:
- Support for other DNS record types (MX, TXT, etc.)
- Caching of DNS responses
- Custom DNS server implementation

## Design Decisions

### DNS Query Approach
- Used `dnspython` library for constructing and parsing DNS messages
- Implemented separate functions for iterative and recursive lookups

### Error Handling
- Implemented timeout mechanism for DNS queries
- Graceful handling of "NXDOMAIN" and other DNS-specific errors

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
   - Handles different response types (A, CNAME, NS)

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

## Testing

### Correctness Testing
- Tested with various domain names (existing and non-existing)
- Verified correct resolution of A and CNAME records
- Checked handling of invalid inputs and network errors

### Performance Testing
- Compared execution time of iterative vs recursive lookups
- Tested with domains requiring multiple levels of resolution

## Challenges and Solutions

### Handling CNAME Records
- **Problem:** Some domains resolve to CNAME before A record
- **Solution:** Implemented recursive CNAME resolution

### Timeout Handling
- **Problem:** Some DNS servers might not respond
- **Solution:** Implemented query timeout and fallback to other servers

## Contribution of Team Members

| Member | Contribution (%) | Role |
|--------|------------------|------|
| [Name] ([Roll No.]) | 33.33% | [Specific contributions] |
| [Name] ([Roll No.]) | 33.33% | [Specific contributions] |
| [Name] ([Roll No.]) | 33.33% | [Specific contributions] |

## References
- DNS protocol specifications (RFC 1034, 1035)
- `dnspython` library documentation
- Course materials on DNS resolution

## Declaration
We hereby declare that we did not indulge in any plagiarism while completing this assignment.

## Feedback
[Add any feedback about the assignment here]
