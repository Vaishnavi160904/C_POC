# Technical Skills and Stopwords Dataset

## Technical skills
- `data/skills.txt` contains exactly 1000 unique technical skills.
- `data/categories.txt` categorizes all 1000 skills into 26 technical domains.
- Duplicate skills are removed from the master skill list and category mapping.

## Categories
1. Programming Languages
2. Web Frontend
3. Backend and APIs
4. Databases and Data Stores
5. Cloud Platforms
6. DevOps and CI/CD
7. Linux and Systems
8. Networking and Protocols
9. Cybersecurity
10. Data Science and Statistics
11. Machine Learning and AI
12. Big Data and Data Engineering
13. Databases and Querying
14. Testing and Quality
15. Version Control and Collaboration
16. Mobile Development
17. Embedded and IoT
18. Cloud Native and Architecture
19. Software Engineering
20. Enterprise and Integration
21. Dev Tools and IDEs
22. UI UX and Design Technology
23. Networking Hardware and Infrastructure
24. Blockchain and Web3
25. Business Intelligence and Analytics
26. Specialized Engineering

## Stopwords
- `data/stopwords.txt` contains 300 common English and resume-context stopwords.
- Technical terms are intentionally excluded where practical so skill extraction is not weakened.
- The application limit remains `MAX_STOPWORDS = 300`.

## Capacity changes
- `MAX_TECH_SKILLS = 1000`
- `MAX_CATEGORIES = 26`
- `MAX_SKILLS_PER_CATEGORY = 50`
