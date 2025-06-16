#!/usr/bin/env pytest
from pathlib import Path
import requests
import aiohttp
import asyncio
import pytest

@pytest.mark.asyncio
async def test_repeated_post_requests_from_multiple_clients():
    post_url = "http://127.0.0.2:8004/cgi/test.py"
    data = "Test content\n"

    sem = asyncio.Semaphore(10)  # Maksimi 10 samanaikaista pyyntöä

    async def limited_post():
            async with sem:
                async with aiohttp.ClientSession() as session:
                    results = []
                    for _ in range(2):  # Lähetetään 2 POST-pyyntöä per client
                        async with session.post(post_url, data=data) as resp:
                            assert resp.status in (200, 201)
                            results.append(await resp.text())
                    return results

    # 10 clienttia × 2 pyyntöä = 20 POSTia
    tasks = [limited_post() for _ in range(3)]
    results = await asyncio.gather(*tasks)
    for res in results:
        print(res)