WITH hash_count AS (
    SELECT count(*) as count, first_block_hash
    FROM file_info
    GROUP BY first_block_hash
)
SELECT  size, fi.first_block_hash, filepath 
FROM file_info fi
JOIN hash_count hc ON fi.first_block_hash = hc.first_block_hash
WHERE hc.count > 1
ORDER BY fi.size DESC 
LIMIT 1000
OFFSET 0;