#!/usr/bin/env python3
"""
Script to extract text from PDF files in the Reference_Material directory
"""

import os
import sys
from pathlib import Path

try:
    import PyPDF2
    HAS_PYPDF2 = True
except ImportError:
    HAS_PYPDF2 = False

try:
    import pdfplumber
    HAS_PDFPLUMBER = True
except ImportError:
    HAS_PDFPLUMBER = False

try:
    import fitz  # PyMuPDF
    HAS_PYMUPDF = True
except ImportError:
    HAS_PYMUPDF = False

def extract_with_pypdf2(pdf_path):
    """Extract text using PyPDF2"""
    text = ""
    try:
        with open(pdf_path, 'rb') as file:
            pdf_reader = PyPDF2.PdfReader(file)
            for page_num, page in enumerate(pdf_reader.pages):
                text += f"\n--- Page {page_num + 1} ---\n"
                text += page.extract_text()
    except Exception as e:
        return f"Error with PyPDF2: {e}"
    return text

def extract_with_pdfplumber(pdf_path):
    """Extract text using pdfplumber"""
    text = ""
    try:
        with pdfplumber.open(pdf_path) as pdf:
            for page_num, page in enumerate(pdf.pages):
                text += f"\n--- Page {page_num + 1} ---\n"
                page_text = page.extract_text()
                if page_text:
                    text += page_text
    except Exception as e:
        return f"Error with pdfplumber: {e}"
    return text

def extract_with_pymupdf(pdf_path):
    """Extract text using PyMuPDF (fitz)"""
    text = ""
    try:
        doc = fitz.open(pdf_path)
        for page_num in range(len(doc)):
            page = doc[page_num]
            text += f"\n--- Page {page_num + 1} ---\n"
            text += page.get_text()
        doc.close()
    except Exception as e:
        return f"Error with PyMuPDF: {e}"
    return text

def extract_pdf_text(pdf_path):
    """Try different PDF extraction methods"""
    # Try PyMuPDF first (usually best quality)
    if HAS_PYMUPDF:
        print(f"  Trying PyMuPDF...")
        result = extract_with_pymupdf(pdf_path)
        if result and not result.startswith("Error"):
            return result
    
    # Try pdfplumber next
    if HAS_PDFPLUMBER:
        print(f"  Trying pdfplumber...")
        result = extract_with_pdfplumber(pdf_path)
        if result and not result.startswith("Error"):
            return result
    
    # Try PyPDF2 as fallback
    if HAS_PYPDF2:
        print(f"  Trying PyPDF2...")
        result = extract_with_pypdf2(pdf_path)
        if result and not result.startswith("Error"):
            return result
    
    return "ERROR: No PDF extraction library available. Please install: pip install pymupdf pdfplumber PyPDF2"

def main():
    ref_dir = Path("Reference_Material")
    
    if not ref_dir.exists():
        print(f"Error: {ref_dir} directory not found")
        return
    
    # Create output directory
    output_dir = Path("extracted_texts")
    output_dir.mkdir(exist_ok=True)
    
    # Find all PDF files
    pdf_files = sorted(ref_dir.glob("*.pdf"))
    
    if not pdf_files:
        print("No PDF files found")
        return
    
    print(f"Found {len(pdf_files)} PDF files")
    print(f"Available libraries: PyMuPDF={HAS_PYMUPDF}, pdfplumber={HAS_PDFPLUMBER}, PyPDF2={HAS_PYPDF2}")
    print()
    
    for pdf_file in pdf_files:
        print(f"Processing: {pdf_file.name}")
        text = extract_pdf_text(pdf_file)
        
        # Save to file
        output_file = output_dir / f"{pdf_file.stem}.txt"
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(text)
        
        print(f"  Saved to: {output_file}")
        print(f"  Extracted {len(text)} characters")
        print()

if __name__ == "__main__":
    main()

