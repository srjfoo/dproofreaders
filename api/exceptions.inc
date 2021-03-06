<?php

// Base exception that all API exceptions should derive from
class ApiException extends Exception
{
    public function getStatusCode()
    {
        return 400;
    }
}

//---------------------------------------------------------------------------
// Common / expected errors we include in the documentation

class BadRequest extends ApiException
{
}

class UnauthorizedError extends ApiException
{
    public function __construct()
    {
        parent::__construct("Unauthorized");
    }

    public function getStatusCode()
    {
        return 401;
    }
}

class NotFoundError extends ApiException
{
    public function __construct($message = "Object not found")
    {
        parent::__construct($message);
    }

    public function getStatusCode()
    {
        return 404;
    }
}

class RateLimitExceeded extends ApiException
{
    public function getStatusCode()
    {
        return 429;
    }
}

//---------------------------------------------------------------------------
// Exceptions that shouldn't happen unless someone is futzing with something
// they shouldn't be, or there's an unexpected problem.

class UnexpectedError extends ApiException
{
}

class InvalidAPI extends ApiException
{
    public function __construct()
    {
        parent::__construct("Invalid API path");
    }

    public function getStatusCode()
    {
        return 404;
    }
}

class MethodNotAllowed extends ApiException
{
    public function __construct()
    {
        parent::__construct("API endpoint doesn't support this method");
    }

    public function getStatusCode()
    {
        return 405;
    }
}

class NotImplementedError extends ApiException
{
    public function __construct()
    {
        parent::__construct("API not implemented");
    }
}

class ServerError extends ApiException
{
    public function __construct()
    {
        parent::__construct("An unhandled error happened on the server");
    }

    public function getStatusCode()
    {
        return 500;
    }
}
