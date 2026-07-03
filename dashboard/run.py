#!/usr/bin/env python
"""Entry point for Smart Factory Dashboard application."""
import sys
import os

# Add dashboard root to path so imports work correctly
dashboard_root = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, dashboard_root)

# Now import and run the app
from app.main import main

if __name__ == "__main__":
    main()
