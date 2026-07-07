# Sri Mart — API Checklist

## Foundation

### GET /api/v1/health
Purpose:
> Tells us whether the server is alive.

Expected:
```json
{
  "status": "UP"
}
```

Status: VERIFIED

### GET /api/v1/hello
Purpose:
> Simple API test proving that routing and JSON response work.

Status: IMPLEMENTED

### GET /api/v1/products
Purpose:
> Returns the current product test data through ProductController.

Status: IMPLEMENTED

## Future Endpoint Checklist

Every future endpoint must document:
- Method: GET/POST/PUT/DELETE
- Route
- Purpose
- Request input
- Response output
- Success status
- Error cases
- Controller
- Service
- Model/data used
- Test performed
- Git phase
